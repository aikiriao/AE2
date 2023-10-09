#include "ae2_simple_hrtf.h"

#include <math.h>
#include <assert.h>
#include <stddef.h>

#include "ae2_delay.h"
#include "ae2_biquad_filter.h"

/* 円周率 */
#define AE2_PI 3.14159265358979323846
/* 15℃の音速 */
#define SPEED_OF_SOUND 340.5
/* メモリアラインメント */
#define AE2SIMPLEHRTF_ALIGNMENT 16
/* nの倍数への切り上げ */
#define AE2SIMPLEHRTF_ROUNDUP(val, n) ((((val) + ((n) - 1)) / (n)) * (n))
/* 水平方向の角度の初期値 */
#define AE2SIMPLEHRTF_DEFAULT_AZIMUTH_ANGLE 0.0

/* HRTFハンドル */
struct AE2SimpleHRTF {
    struct AE2Delay *delay; /* ディレイ */
    struct AE2BiquadFilter filter; /* 頭部伝達関数を表現する1次フィルタ */
    float sampling_rate; /* サンプリングレート */
    float radius_of_head; /* 実効頭半径 */
    float azimuth_angle; /* 水平角度 */
    float delay_fade_time_ms; /* ディレイのフェード時間 */
    float max_radius_of_head; /* 最大の頭半径 */
    int32_t max_num_process_samples; /* 最大処理サンプル数 */
};

/* パラメータ設定 */
static void AE2SimpleHRTF_SetParameter(struct AE2SimpleHRTF *hrtf, float radius, float angle);

/* ディレイ作成に必要なワークサイズ計算 */
int32_t AE2SimpleHRTF_CalculateWorkSize(const struct AE2SimpleHRTFConfig *config)
{
    int32_t work_size;

    /* 引数チェック */
    if (config == NULL) {
        return -1;
    }

    /* コンフィグチェック */
    if ((config->delay_fade_time_ms < 0.0f) || (config->max_radius_of_head < (float)AE2SIMPLEHRTF_DEFAULT_RADIUS_OF_HEAD)
        || (config->sampling_rate <= 0.0f) || (config->max_num_process_samples <= 0)) {
        return -1;
    }

    /* 構造体サイズを計算 */
    work_size = sizeof(struct AE2SimpleHRTF) + AE2SIMPLEHRTF_ALIGNMENT;

    /* ディレイの領域サイズ計算 */
    {
        int32_t delay_size;
        struct AE2DelayConfig delay_config;
        delay_config.max_num_delay_samples
            = (int32_t)ceil((config->max_radius_of_head / SPEED_OF_SOUND) * config->sampling_rate * (AE2_PI + 1.0));
        delay_config.max_num_process_samples = config->max_num_process_samples;
        if ((delay_size = AE2Delay_CalculateWorkSize(&delay_config)) < 0) {
            return -1;
        }
        work_size += delay_size;
    }

    return work_size;
}

/* ディレイ作成 */
struct AE2SimpleHRTF *AE2SimpleHRTF_Create(const struct AE2SimpleHRTFConfig *config, void *work, int32_t work_size)
{
    struct AE2SimpleHRTF *hrtf;
    uint8_t *work_ptr;

    /* 引数チェック */
    if ((config == NULL) || (work == NULL) || (work_size < 0)) {
        return NULL;
    }

    if (work_size < AE2SimpleHRTF_CalculateWorkSize(config)) {
        return NULL;
    }

    /* ハンドル領域割当 */
    work_ptr = (uint8_t *)AE2SIMPLEHRTF_ROUNDUP((uintptr_t)work, AE2SIMPLEHRTF_ALIGNMENT);
    hrtf = (struct AE2SimpleHRTF *)work_ptr;
    hrtf->max_radius_of_head = config->max_radius_of_head;
    hrtf->sampling_rate = config->sampling_rate;
    hrtf->max_num_process_samples = config->max_num_process_samples;
    hrtf->delay_fade_time_ms = config->delay_fade_time_ms;
    work_ptr += sizeof(struct AE2SimpleHRTF);

    /* ディレイの領域割り当て */
    {
        int32_t delay_size;
        struct AE2DelayConfig delay_config;
        delay_config.max_num_delay_samples
            = (int32_t)ceil((config->max_radius_of_head / SPEED_OF_SOUND) * config->sampling_rate * (AE2_PI + 1.0));
        delay_config.max_num_process_samples = config->max_num_process_samples;
        if ((delay_size = AE2Delay_CalculateWorkSize(&delay_config)) < 0) {
            return NULL;
        }
        if ((hrtf->delay = AE2Delay_Create(&delay_config, work_ptr, delay_size)) == NULL) {
            return NULL;
        }
        work_ptr += delay_size;
    }

    /* 内部バッファリセット */
    AE2SimpleHRTF_Reset(hrtf);

    /* 初期状態では実効半径0.08f、かつ正面にあるものとして係数計算 */
    AE2SimpleHRTF_SetParameter(hrtf, AE2SIMPLEHRTF_DEFAULT_RADIUS_OF_HEAD, AE2SIMPLEHRTF_DEFAULT_AZIMUTH_ANGLE);
    hrtf->radius_of_head = AE2SIMPLEHRTF_DEFAULT_RADIUS_OF_HEAD;
    hrtf->azimuth_angle = AE2SIMPLEHRTF_DEFAULT_AZIMUTH_ANGLE;

    return hrtf;
}

/* ディレイ破棄 */
void AE2SimpleHRTF_Destroy(struct AE2SimpleHRTF *hrtf)
{
    assert(hrtf != NULL);

    /* 不定領域アクセス防止のため内容はクリア */
    AE2SimpleHRTF_Reset(hrtf);
}

/* リセット */
void AE2SimpleHRTF_Reset(struct AE2SimpleHRTF *hrtf)
{
    assert(hrtf != NULL);

    /* バッファの内容をクリア */
    AE2Delay_Reset(hrtf->delay);
    AE2BiquadFilter_ClearBuffer(&(hrtf->filter));
}

/* パラメータ設定 */
static void AE2SimpleHRTF_SetParameter(struct AE2SimpleHRTF *hrtf, float radius, float angle)
{
    float num_delay_samples;
    /* 最小角(150°) */
    const double theta_min = (5.0 * AE2_PI) / 6.0;
    /* alphaの最小値 */
    const double alpha_min = 0.01;
    /* 周波数パラメータ */
    const double omega0 = SPEED_OF_SOUND / radius;

    /* リスナー正面からの角度に直す */
    // angle += AE2_PI / 2.0;

    /* IIRフィルタ係数設定 */
    {
        const double alpha = 1.0 + alpha_min / 2.0 + (1.0 - alpha_min / 2.0) * cos(angle / theta_min * AE2_PI);
        const double a0 = omega0 + hrtf->sampling_rate;
        hrtf->filter.a1 = (float)((omega0 - hrtf->sampling_rate) / a0);
        hrtf->filter.b0 = (float)((omega0 + alpha * hrtf->sampling_rate) / a0);
        hrtf->filter.b1 = (float)((omega0 - alpha * hrtf->sampling_rate) / a0);
        hrtf->filter.a2 = hrtf->filter.b2 = 0.0f;
    }

    /* 遅延サンプル数設定 */
    if (fabs(angle) < AE2_PI / 2.0) {
        num_delay_samples = -(hrtf->sampling_rate / omega0) * (cos(angle) - 1.0);
    } else {
        num_delay_samples = (hrtf->sampling_rate / omega0) * ((fabs(angle) - AE2_PI / 2.0) + 1.0);
    }

    /* フェード時間後に遅延サンプルに達するように設定 */
    AE2Delay_SetDelay(hrtf->delay, num_delay_samples,
        AE2DELAY_FADETYPE_LINEAR, (int32_t)(hrtf->delay_fade_time_ms * hrtf->sampling_rate / 1000.0));
}

/* 実効頭部半径の設定 */
void AE2SimpleHRTF_SetHeadRadius(struct AE2SimpleHRTF *hrtf, float radius)
{
    assert(hrtf != NULL);
    assert(radius > 0.0f);

    /* パラメータ設定 */
    AE2SimpleHRTF_SetParameter(hrtf, radius, hrtf->azimuth_angle);

    /* 設定値を記録 */
    hrtf->radius_of_head = radius;
}

/* 水平方向角度の設定 */
void AE2SimpleHRTF_SetAzimuthAngle(struct AE2SimpleHRTF *hrtf, float angle)
{
    assert(hrtf != NULL);
    assert((angle >= -(float)AE2_PI) && (angle <= (float)AE2_PI));

    AE2SimpleHRTF_SetParameter(hrtf, hrtf->radius_of_head, angle);

    /* 設定値を記録 */
    hrtf->azimuth_angle = angle;
}

/* HRTF適用 */
void AE2SimpleHRTF_Process(
    struct AE2SimpleHRTF *hrtf, float *data, uint32_t num_samples)
{
    assert(hrtf != NULL);
    assert(data != NULL);
    assert(num_samples != 0);

    /* 初期遅延適用 */
    AE2Delay_Process(hrtf->delay, data, num_samples);

    /* HRTF適用 */
    AE2BiquadFilter_Process(&hrtf->filter, data, num_samples);
}

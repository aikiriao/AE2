#include "ae2_delay.h"

#include <string.h>
#include <assert.h>

#include "ae2_ring_buffer.h"

/* メモリアラインメント */
#define AE2DELAY_ALIGNMENT 16
/* nの倍数への切り上げ */
#define AE2DELAY_ROUNDUP(val, n) ((((val) + ((n) - 1)) / (n)) * (n))
/* 最小値の取得 */
#define AE2DELAY_MIN(a,b) (((a) < (b)) ? (a) : (b))
/* 最大値の取得 */
#define AE2DELAY_MAX(a,b) (((a) > (b)) ? (a) : (b))

/* ディレイハンドル */
struct AE2Delay {
    struct AE2RingBuffer *buffer; /* リングバッファ */
    float fade_ratio; /* 遅延量フェードの割合を管理するレシオ */
    float delta_ratio; /* レシオ増分 */
    int32_t max_num_process_samples; /* 最大処理サンプル数 */
    int32_t max_num_delay_samples; /* 最大遅延サンプル数 */
    AE2DelayFadeType fade_type; /* フェードの種類 */
    int32_t delay_num_samples; /* 現在の遅延量 */
    int32_t prev_delay_num_samples; /* 前の遅延量 */
};

/* ディレイ作成に必要なワークサイズ計算 */
int32_t AE2Delay_CalculateWorkSize(const struct AE2DelayConfig *config)
{
    int32_t work_size;

    /* 引数チェック */
    if (config == NULL) {
        return -1;
    }

    /* 構造体サイズを計算 */
    work_size = sizeof(struct AE2Delay) + AE2DELAY_ALIGNMENT;

    /* ディレイバッファの領域サイズ計算 */
    {
        int32_t buffer_size;
        struct AE2RingBufferConfig buffer_config;
        /* 遅延分の保持と、最大遅延分遡れるように2倍確保 */
        buffer_config.max_ndata = AE2DELAY_MAX(2 * config->max_num_delay_samples, config->max_num_process_samples);
        buffer_config.max_required_ndata = config->max_num_process_samples;
        buffer_config.data_unit_size = sizeof(float);
        if ((buffer_size = AE2RingBuffer_CalculateWorkSize(&buffer_config)) < 0) {
            return -1;
        }
        work_size += buffer_size;
    }

    return work_size;
}

/* ディレイ作成 */
struct AE2Delay *AE2Delay_Create(const struct AE2DelayConfig *config, void *work, int32_t work_size)
{
    struct AE2Delay *delay;
    uint8_t *work_ptr;

    /* 引数チェック */
    if ((config == NULL) || (work == NULL) || (work_size < 0)) {
        return NULL;
    }

    if (work_size < AE2Delay_CalculateWorkSize(config)) {
        return NULL;
    }

    /* ハンドル領域割当 */
    work_ptr = (uint8_t *)AE2DELAY_ROUNDUP((uintptr_t)work, AE2DELAY_ALIGNMENT);
    delay = (struct AE2Delay *)work_ptr;
    delay->max_num_process_samples = config->max_num_process_samples;
    delay->max_num_delay_samples = config->max_num_delay_samples;
    work_ptr += sizeof(struct AE2Delay);

    /* ディレイバッファの領域割り当て */
    {
        int32_t buffer_size;
        struct AE2RingBufferConfig buffer_config;
        buffer_config.max_ndata = AE2DELAY_MAX(2 * config->max_num_delay_samples, config->max_num_process_samples);
        buffer_config.max_required_ndata = config->max_num_process_samples;
        buffer_config.data_unit_size = sizeof(float);
        if ((buffer_size = AE2RingBuffer_CalculateWorkSize(&buffer_config)) < 0) {
            return NULL;
        }
        if ((delay->buffer = AE2RingBuffer_Create(&buffer_config, work_ptr, buffer_size)) == NULL) {
            return NULL;
        }
        work_size += buffer_size;
    }

    return delay;
}

/* ディレイ破棄 */
void AE2Delay_Destroy(struct AE2Delay *delay)
{
    assert(delay != NULL);

    /* 不定領域アクセス防止のため内容はクリア */
    AE2Delay_Reset(delay);
}

void AE2Delay_Reset(struct AE2Delay *delay)
{
    assert(delay != NULL);

    /* バッファの内容をクリア */
    AE2RingBuffer_Clear(delay->buffer);

    /* フェードの進捗をクリア */
    delay->fade_ratio = 0.0f;

    /* 遅延量をクリア */
    delay->delay_num_samples = delay->prev_delay_num_samples = 0;
}

/* 遅延量を設定 */
void AE2Delay_SetDelay(struct AE2Delay *delay,
        int32_t num_delay_samples, AE2DelayFadeType fade_type, int32_t num_fade_samples)
{
    assert(delay != NULL);
    assert(num_delay_samples >= 0);
    assert(num_fade_samples >= 0);
    assert(num_delay_samples <= delay->max_num_delay_samples);

    /* 遅延量を更新 */
    delay->prev_delay_num_samples = delay->delay_num_samples;
    delay->delay_num_samples = num_delay_samples;

    /* フェードの情報をリセット */
    if (num_fade_samples == 0) {
        delay->fade_ratio = 1.0f;
        delay->delta_ratio = 1.0f;
    } else {
        if (delay->fade_ratio >= 1.0f) {
            delay->fade_ratio = 0.0f;
        }
        /* フェード中ならば、現在のレシオを保ちつつも次の増分を決める */
        delay->delta_ratio = (1.0f - delay->fade_ratio) / num_fade_samples;
    }
    delay->fade_type = fade_type;
}

/* 線形補間による遅延フェード */
static void AE2Delay_ProcessLinearFade(
    struct AE2Delay *delay, const float *prev_delay, const float *target_delay, float *output, int32_t num_samples)
{
    int32_t smpl;
    const float delta = delay->delta_ratio;
    float ratio = delay->fade_ratio;
    const int32_t num_remain_samples = (int32_t)AE2DELAY_MIN(num_samples, (1.0 - ratio) / delta);

    assert(delay != NULL);
    assert(prev_delay != NULL);
    assert(target_delay != NULL);
    assert(output != NULL);

    for (smpl = 0; smpl < num_remain_samples; smpl++) {
        output[smpl] = ratio * target_delay[smpl] + (1.0f - ratio) * prev_delay[smpl];
        ratio += delta;
    }
    if (smpl < num_samples) {
        memcpy(&output[smpl], &target_delay[smpl], sizeof(float) * (num_samples - smpl));
    }

    delay->fade_ratio = ratio;
}

/* 線形補間による遅延フェード */
static void AE2Delay_ProcessSquareFade(
    struct AE2Delay *delay, const float *prev_delay, const float *target_delay, float *output, int32_t num_samples)
{
    int32_t smpl;
    const float delta = delay->delta_ratio;
    float ratio = delay->fade_ratio;
    const int32_t num_remain_samples = (int32_t)AE2DELAY_MIN(num_samples, (1.0 - ratio) / delta);

    assert(delay != NULL);
    assert(prev_delay != NULL);
    assert(target_delay != NULL);
    assert(output != NULL);

    for (smpl = 0; smpl < num_remain_samples; smpl++) {
        const float square_ratio = ratio * ratio;
        output[smpl] = square_ratio * target_delay[smpl] + (1.0f - square_ratio) * prev_delay[smpl];
        ratio += delta;
    }
    if (smpl < num_samples) {
        memcpy(&output[smpl], &target_delay[smpl], sizeof(float) * (num_samples - smpl));
    }

    delay->fade_ratio = ratio;
}

/* 信号処理 */
void AE2Delay_Process(struct AE2Delay *delay, float *data, int32_t num_samples)
{
    int32_t smpl;
    float *target_delay, *prev_delay;

    assert(delay != NULL);
    assert(data != NULL);
    assert(num_samples <= delay->max_num_process_samples);

    /* バッファにデータを挿入 */
    AE2RingBuffer_Put(delay->buffer, data, num_samples);

    /* 変更前の遅延と変更後の遅延を取得 */
    AE2RingBuffer_DelayedPeek(delay->buffer, (void **)&prev_delay, num_samples, delay->prev_delay_num_samples);
    AE2RingBuffer_DelayedPeek(delay->buffer, (void **)&target_delay, num_samples, delay->delay_num_samples);

    /* ミックスしながら出力 */
    switch (delay->fade_type) {
    case AE2DELAY_FADETYPE_LINEAR:
        AE2Delay_ProcessLinearFade(delay, prev_delay, target_delay, data, num_samples);
        break;
    case AE2DELAY_FADETYPE_SQUARE:
        AE2Delay_ProcessSquareFade(delay, prev_delay, target_delay, data, num_samples);
        break;
    default:
        assert(0);
    }

    /* 出力したサンプル分空読み */
    AE2RingBuffer_Get(delay->buffer, (void **)&target_delay, num_samples);
}

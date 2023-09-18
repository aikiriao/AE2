#include "ae2_iir_filter.h"

#include <math.h>
#include <assert.h>

#include "ae2_biquad_filter.h"

/* 円周率 */
#define AE2_PI 3.14159265358979323846
/* アラインメント */
#define AE2IIFILTER_ALIGNMENT 16
/* nの倍数への切り上げ */
#define AE2IIRFILTER_ROUNDUP(val, n) ((((val) + ((n) - 1)) / (n)) * (n))
/* バイクアッドフィルタの個数を取得 */
#define AE2IIRFILTER_NUM_BIQUAD_FILTERS(order) (((order) + 1) / 2)

/*!
* @brief IIRフィルタ構造体
*/
struct AE2IIRFilter {
    int32_t max_order; /*!< 最大次数 */
    int32_t order; /*!< 現在セットされている次数 */
    int32_t num_biquad_filters; /*!< バイクアッドフィルタ数 */
    struct AE2BiquadFilter *biquad_filter; /*!< バイクアッドフィルタ列 */
};

/* 生成に必要なワークサイズ計算 */
int32_t AE2IIRFilter_CalculateWorkSize(int32_t max_order)
{
    int32_t work_size;

    /* 引数チェック */
    if (max_order <= 0) {
        return -1;
    }

    /* 構造体サイズ */
    work_size = AE2IIFILTER_ALIGNMENT + sizeof(struct AE2IIRFilter);

    /* バイクアッドフィルタのサイズ */
    work_size += AE2IIFILTER_ALIGNMENT + AE2IIRFILTER_NUM_BIQUAD_FILTERS(max_order) * sizeof(struct AE2BiquadFilter);

    return work_size;
}

/* IIRフィルタ生成 */
struct AE2IIRFilter *AE2IIRFilter_Create(int32_t max_order, void *work, int32_t work_size)
{
    int32_t i;
    struct AE2IIRFilter *filter;
    uint8_t *work_ptr;

    /* 引数チェック */
    if ((max_order <= 0) || (work == NULL) || (work_size < 0)) {
        return NULL;
    }

    if (work_size < AE2IIRFilter_CalculateWorkSize(max_order)) {
        return NULL;
    }

    /* ハンドル領域割当 */
    work_ptr = (uint8_t *)AE2IIRFILTER_ROUNDUP((uintptr_t)work, AE2IIFILTER_ALIGNMENT);
    filter = (struct AE2IIRFilter *)work_ptr;
    work_ptr += sizeof(struct AE2IIRFilter);

    /* 構造体変数初期化 */
    filter->max_order = max_order;
    filter->order = 0;
    filter->num_biquad_filters = 0;

    /* バイクアッドフィルタ領域割り当て */
    {
        const int32_t max_num_biquad_filters = AE2IIRFILTER_NUM_BIQUAD_FILTERS(max_order);
        work_ptr = (uint8_t *)AE2IIRFILTER_ROUNDUP((uintptr_t)work_ptr, AE2IIFILTER_ALIGNMENT);
        filter->biquad_filter = (struct AE2BiquadFilter *)work_ptr;
        work_ptr += max_num_biquad_filters * sizeof(struct AE2BiquadFilter);

        /* バッファクリア */
        for (i = 0; i < max_num_biquad_filters; i++) {
            AE2BiquadFilter_ClearBuffer(&filter->biquad_filter[i]);
        }
    }

    assert((int32_t)(work_ptr - (uint8_t *)work) <= work_size);

    return filter;
}

/* IIRフィルタ破棄 */
void AE2IIRFilter_Destroy(struct AE2IIRFilter *filter)
{
    /* 特に何もしない */
}

/* バッファクリア */
void AE2IIRFilter_ClearBuffer(struct AE2IIRFilter *filter)
{
    int32_t i;

    /* 引数チェック */
    assert(filter != NULL);

    /* 全フィルタのバッファをクリア */
    for (i = 0; i < filter->num_biquad_filters; i++) {
        AE2BiquadFilter_ClearBuffer(&filter->biquad_filter[i]);
    }
}

/* フィルタ適用 */
void AE2IIRFilter_Process(
        struct AE2IIRFilter *filter, float *data, uint32_t num_samples)
{
    int32_t i;

    /* 引数チェック */
    assert(filter != NULL);

    /* 全フィルタを適用 */
    for (i = 0; i < filter->num_biquad_filters; i++) {
        AE2BiquadFilter_Process(&filter->biquad_filter[i], data, num_samples);
    }
}

/* 正規化角周波数と共役な極からローパスバイクアッドフィルタ係数を設定 */
static void AE2IIRFilter_SetBiquadLowPassByConjugatePole(
    struct AE2BiquadFilter *filter, double omega0, double re_pole, double im_pole)
{
    const double sinw0 = sin(omega0);
    const double cosw0 = cos(omega0);
    const double pow_pole = re_pole * re_pole + im_pole * im_pole;
    const double a0 = (1.0 + cosw0) - 2.0 * re_pole * sinw0 + pow_pole * (1.0 - cosw0);

    assert(filter != NULL);

    filter->a1 = (float)((2.0 * (pow_pole * (1.0 - cosw0) - (1.0 + cosw0))) / a0);
    filter->a2 = (float)(((1.0 + cosw0) + 2.0 * re_pole * sinw0 + pow_pole * (1.0 - cosw0)) / a0);
    filter->b0 = (float)((1.0 - cosw0) / a0);
    filter->b1 = (float)(2.0 * (1.0 - cosw0) / a0);
    filter->b2 = (float)((1.0 - cosw0) / a0);
}

/* 正規化角周波数と実軸上の極から1次のローパスバイクアッドフィルタ係数を設定 */
static void AE2IIRFilter_Set1stOrderLowPassByReal(struct AE2BiquadFilter *filter, double omega0, double re_pole)
{
    const double sinw0 = sin(omega0);
    const double cosw0 = cos(omega0);
    const double a0 = sinw0 - re_pole * (1.0 - cosw0);

    assert(filter != NULL);
    assert(re_pole <= 0.0); /* 安定条件 */

    filter->a1 = (float)(-(sinw0 + re_pole * (1.0 - cosw0)) / a0);
    filter->b0 = (float)((1.0 - cosw0) / a0);
    filter->b1 = (float)((1.0 - cosw0) / a0);

    filter->a2 = filter->b2 = 0.0f;
}

/* 正規化角周波数と共役な極からハイパスバイクアッドフィルタ係数を設定 */
static void AE2IIRFilter_SetBiquadHighPassByConjugatePole(
    struct AE2BiquadFilter *filter, double omega0, double re_pole, double im_pole)
{
    const double sinw0 = sin(omega0);
    const double cosw0 = cos(omega0);
    const double pow_pole = re_pole * re_pole + im_pole * im_pole;
    const double a0 = pow_pole * (1.0 + cosw0) - 2.0 * re_pole * sinw0 + (1.0 - cosw0);

    assert(filter != NULL);

    filter->a1 = (float)((2.0 * ((1.0 - cosw0) - pow_pole * (1.0 + cosw0))) / a0);
    filter->a2 = (float)((pow_pole * (1.0 + cosw0) + 2.0 * re_pole * sinw0 + (1.0 - cosw0)) / a0);
    filter->b0 = (float)((1.0 + cosw0) / a0);
    filter->b1 = (float)(-2.0 * (1.0 + cosw0) / a0);
    filter->b2 = (float)((1.0 + cosw0) / a0);
}

/* 正規化角周波数と実軸上の極から1次のハイパスバイクアッドフィルタ係数を設定 */
static void AE2IIRFilter_Set1stOrderHighPassByReal(struct AE2BiquadFilter *filter, double omega0, double re_pole)
{
    const double sinw0 = sin(omega0);
    const double cosw0 = cos(omega0);
    const double a0 = -re_pole * sinw0 + (1.0 - cosw0);

    assert(filter != NULL);
    assert(re_pole <= 0.0); /* 安定条件 */

    filter->a1 = (float)((re_pole * sinw0 + (1.0 - cosw0)) / a0);
    filter->b0 = (float)(sinw0 / a0);
    filter->b1 = (float)(-sinw0 / a0);

    filter->a2 = filter->b2 = 0.0f;
}

/* バターワースフィルタ設定の共通ルーチン */
static void AE2IIRFilter_SetButterworthCommon(
    struct AE2IIRFilter *filter,
    int32_t order, float sampling_rate, float frequency,
    void (*biquad_setter)(struct AE2BiquadFilter *, double, double, double),
    void (*first_order_setter)(struct AE2BiquadFilter *, double, double))
{
    int32_t i;

    /* 引数チェック */
    assert(filter != NULL);
    assert(filter->max_order >= order);
    assert(sampling_rate > 0.0);
    assert(frequency >= 0.0);
    assert(biquad_setter != NULL);
    assert(first_order_setter != NULL);

    /* 正規化カットオフ周波数 */
    const double omega0 = 2.0 * AE2_PI * frequency / sampling_rate;

    /* バイクアッドフィルタに係数設定 */
    for (i = 0; i < order / 2; i++) {
        struct AE2BiquadFilter *f = &filter->biquad_filter[i];
        const double theta_k = AE2_PI * (2.0 * (i + 1.0) + order - 1.0) / (2.0 * order);
        biquad_setter(f, omega0, cos(theta_k), sin(theta_k));
    }

    /* 奇数の場合、最後のフィルタは1次フィルタ */
    if (order % 2 == 1) {
        first_order_setter(&filter->biquad_filter[order / 2], omega0, -1.0);
    }

    /* バイクアッドフィルタ個数の設定 */
    filter->num_biquad_filters = AE2IIRFILTER_NUM_BIQUAD_FILTERS(order);
}

/* ローパス特性のバターワースフィルタを設定 */
void AE2IIRFilter_SetButterworthLowPass(
    struct AE2IIRFilter *filter, int32_t order, float sampling_rate, float frequency)
{
    /* 引数チェック */
    assert(filter != NULL);
    assert(filter->max_order >= order);
    assert(sampling_rate > 0.0);
    assert(frequency >= 0.0);

    /* 共通ルーチンを使用して設計 */
    AE2IIRFilter_SetButterworthCommon(
        filter, order, sampling_rate, frequency,
        AE2IIRFilter_SetBiquadLowPassByConjugatePole, AE2IIRFilter_Set1stOrderLowPassByReal);
}

/* ハイパス特性のチェビシェフフィルタを設定 */
void AE2IIRFilter_SetButterworthHighPass(
    struct AE2IIRFilter *filter, int32_t order, float sampling_rate, float frequency)
{
    /* 引数チェック */
    assert(filter != NULL);
    assert(filter->max_order >= order);
    assert(sampling_rate > 0.0);
    assert(frequency >= 0.0);

    /* 共通ルーチンを使用して設計 */
    AE2IIRFilter_SetButterworthCommon(
        filter, order, sampling_rate, frequency,
        AE2IIRFilter_SetBiquadHighPassByConjugatePole, AE2IIRFilter_Set1stOrderHighPassByReal);
}

/* チェビシェフフィルタ設定の共通ルーチン */
static void AE2IIRFilter_SetChebyShevCommon(
    struct AE2IIRFilter *filter,
    int32_t order, float sampling_rate, float frequency, float ripple,
    void (*biquad_setter)(struct AE2BiquadFilter *, double, double, double),
    void (*first_order_setter)(struct AE2BiquadFilter *, double, double))
{
    int32_t i;

    /* 引数チェック */
    assert(filter != NULL);
    assert(filter->max_order >= order);
    assert(sampling_rate > 0.0);
    assert(frequency >= 0.0);
    assert(biquad_setter != NULL);
    assert(first_order_setter != NULL);

    /* バイクアッドフィルタ数 */
    const int32_t tmp_num_biquad_filters = AE2IIRFILTER_NUM_BIQUAD_FILTERS(order);
    /* リップルをεに変換 */
    const double epsilon = sqrt(pow(10.0, ripple / 10.0) - 1.0);
    /* 正規化カットオフ周波数 */
    const double omega0 = 2.0 * AE2_PI * frequency / sampling_rate;
    /* 計算用定数 */
    const double b = asinh(1.0 / epsilon) / order;

    /* バイクアッドフィルタに係数設定 */
    for (i = 0; i < order / 2; i++) {
        /* 単位円から遠い（発散しずらい）極から処理していくため、末尾から設定 */
        struct AE2BiquadFilter *f = &filter->biquad_filter[tmp_num_biquad_filters - i - 1];
        const double theta_k = AE2_PI * (2.0 * (i + 1.0) + order - 1.0) / (2.0 * order);
        const double re_pk = cos(theta_k) * sinh(b);
        const double im_pk = sin(theta_k) * cosh(b);
        biquad_setter(f, omega0, re_pk, im_pk);
    }

    /* 奇数の場合、最初のフィルタは1次フィルタ */
    if (order % 2 == 1) {
        first_order_setter(&filter->biquad_filter[0], omega0, -sinh(b));
    }

    /* 通過域のゲインが1になるように先頭のフィルタで補正 */
    {
        struct AE2BiquadFilter *f = &filter->biquad_filter[0];
        /* 補正ゲイン */
        const double gain = pow(2.0, 1.0 - order) / epsilon;
        f->b0 *= gain;
        f->b1 *= gain;
        f->b2 *= gain;
    }

    /* バイクアッドフィルタ個数の設定 */
    filter->num_biquad_filters = AE2IIRFILTER_NUM_BIQUAD_FILTERS(order);
}

/* ローパス特性のチェビシェフフィルタを設定 */
void AE2IIRFilter_SetChebyshevLowPass(
    struct AE2IIRFilter *filter,
    int32_t order, float sampling_rate, float frequency, float ripple)
{
    /* 引数チェック */
    assert(filter != NULL);
    assert(filter->max_order >= order);
    assert(sampling_rate > 0.0);
    assert(frequency >= 0.0);

    /* 共通ルーチンを使用して設計 */
    AE2IIRFilter_SetChebyShevCommon(
        filter, order, sampling_rate, frequency, ripple,
        AE2IIRFilter_SetBiquadLowPassByConjugatePole, AE2IIRFilter_Set1stOrderLowPassByReal);
}

/* ハイパス特性のチェビシェフフィルタを設定 */
void AE2IIRFilter_SetChebyshevHighPass(
    struct AE2IIRFilter *filter,
    int32_t order, float sampling_rate, float frequency, float ripple)
{
    /* 引数チェック */
    assert(filter != NULL);
    assert(filter->max_order >= order);
    assert(sampling_rate > 0.0);
    assert(frequency >= 0.0);

    /* 共通ルーチンを使用して設計 */
    AE2IIRFilter_SetChebyShevCommon(
        filter, order, sampling_rate, frequency, ripple,
        AE2IIRFilter_SetBiquadHighPassByConjugatePole, AE2IIRFilter_Set1stOrderHighPassByReal);
}

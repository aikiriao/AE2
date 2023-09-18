#include "ae2_biquad_filter.h"

#include <math.h>
#include <assert.h>
#include <stddef.h>

/* 円周率 */
#define AE2_PI 3.14159265358979323846

/* ローパスフィルタの設定 */
void AE2BiquadFilter_SetLowPass(
    struct AE2BiquadFilter *filter, float sampling_rate, float frequency, float quality_factor, float gain)
{
    double w0, alpha, sinw0, cosw0, a0;

    /* 引数チェック */
    assert(filter != NULL);
    assert(sampling_rate > 0.0f);
    assert(frequency > 0.0f);
    assert(quality_factor > 0.0f);
    assert(gain > 0.0f);

    /* フィルタ設計 */
    w0 = 2.0 * AE2_PI * frequency / sampling_rate;
    sinw0 = sin(w0);
    cosw0 = cos(w0);
    alpha = sinw0 / (2.0 * quality_factor);
    a0 = 1.0 + alpha;

    filter->b0 = (float)(((1.0 - cosw0) * gain / 2.0) / a0);
    filter->b1 = (float)((1.0 - cosw0) * gain / a0);
    filter->b2 = (float)(((1.0 - cosw0) * gain / 2.0) / a0);
    filter->a1 = (float)((-2.0 * cosw0) / a0);
    filter->a2 = (float)((1.0 - alpha) / a0);
}

/* ハイパスフィルタの設定 */
void AE2BiquadFilter_SetHighPass(
    struct AE2BiquadFilter *filter, float sampling_rate, float frequency, float quality_factor, float gain)
{
    double w0, alpha, sinw0, cosw0, a0;

    /* 引数チェック */
    assert(filter != NULL);
    assert(sampling_rate > 0.0f);
    assert(frequency > 0.0f);
    assert(quality_factor > 0.0f);
    assert(gain > 0.0f);

    /* フィルタ設計 */
    w0 = 2.0 * AE2_PI * frequency / sampling_rate;
    sinw0 = sin(w0);
    cosw0 = cos(w0);
    alpha = sinw0 / (2.0 * quality_factor);
    a0 = 1.0 + alpha;

    filter->b0 = (float)(((1.0 + cosw0) * gain / 2.0) / a0);
    filter->b1 = (float)(-(1.0 + cosw0) * gain / a0);
    filter->b2 = (float)(((1.0 + cosw0) * gain / 2.0) / a0);
    filter->a1 = (float)((-2.0 * cosw0) / a0);
    filter->a2 = (float)((1.0 - alpha) / a0);
}

/* バンドパスフィルタの設定 */
void AE2BiquadFilter_SetBandPass(
    struct AE2BiquadFilter *filter, float sampling_rate, float frequency, float quality_factor, float gain)
{
    double w0, alpha, sinw0, cosw0, a0;

    /* 引数チェック */
    assert(filter != NULL);
    assert(sampling_rate > 0.0f);
    assert(frequency > 0.0f);
    assert(quality_factor > 0.0f);
    assert(gain > 0.0f);

    /* フィルタ設計 */
    w0 = 2.0 * AE2_PI * frequency / sampling_rate;
    sinw0 = sin(w0);
    cosw0 = cos(w0);
    alpha = sinw0 / (2.0 * quality_factor);
    a0 = 1.0 + alpha;

    filter->b0 = (float)(alpha * gain / a0);
    filter->b1 = 0.0f;
    filter->b2 = (float)(-alpha * gain / a0);
    filter->a1 = (float)((-2.0 * cosw0) / a0);
    filter->a2 = (float)((1.0 - alpha) / a0);
}

/* バンドエリミネート（リジェクト、ノッチ）フィルタの設定 */
void AE2BiquadFilter_SetBandEliminate(
    struct AE2BiquadFilter *filter, float sampling_rate, float frequency, float quality_factor, float gain)
{
    double w0, alpha, sinw0, cosw0, a0;

    /* 引数チェック */
    assert(filter != NULL);
    assert(sampling_rate > 0.0f);
    assert(frequency > 0.0f);
    assert(quality_factor > 0.0f);
    assert(gain > 0.0f);

    /* フィルタ設計 */
    w0 = 2.0 * AE2_PI * frequency / sampling_rate;
    sinw0 = sin(w0);
    cosw0 = cos(w0);
    alpha = sinw0 / (2.0 * quality_factor);
    a0 = 1.0 + alpha;

    filter->b0 = (float)(gain / a0);
    filter->b1 = (float)((-2.0 * cosw0 * gain) / a0);
    filter->b2 = (float)(gain / a0);
    filter->a1 = (float)((-2.0 * cosw0) / a0);
    filter->a2 = (float)((1.0 - alpha) / a0);
}

/* オールパスフィルタの設定 */
void AE2BiquadFilter_SetAllPass(
    struct AE2BiquadFilter *filter, float sampling_rate, float frequency, float quality_factor, float gain)
{
    double w0, alpha, sinw0, cosw0, a0;

    /* 引数チェック */
    assert(filter != NULL);
    assert(sampling_rate > 0.0f);
    assert(frequency > 0.0f);
    assert(quality_factor > 0.0f);
    assert(gain > 0.0f);

    /* フィルタ設計 */
    w0 = 2.0 * AE2_PI * frequency / sampling_rate;
    sinw0 = sin(w0);
    cosw0 = cos(w0);
    alpha = sinw0 / (2.0 * quality_factor);
    a0 = 1.0 + alpha;

    filter->b0 = (float)((1.0 - alpha) * gain / a0);
    filter->b1 = (float)((-2.0 * cosw0) * gain / a0);
    filter->b2 = (float)((1.0 + alpha) * gain / a0);
    filter->a1 = (float)((-2.0 * cosw0) / a0);
    filter->a2 = (float)((1.0 - alpha) / a0);
}

/* ローシェルフフィルタの設定 */
void AE2BiquadFilter_SetLowShelf(
    struct AE2BiquadFilter *filter, float sampling_rate, float frequency, float quality_factor, float gain)
{
    double w0, alpha, A, sinw0, cosw0, a0;

    /* 引数チェック */
    assert(filter != NULL);
    assert(sampling_rate > 0.0f);
    assert(frequency > 0.0f);
    assert(quality_factor > 0.0f);
    assert(gain > 0.0f);

    A = sqrt(gain);
    w0 = 2.0 * AE2_PI * frequency / sampling_rate;
    sinw0 = sin(w0);
    cosw0 = cos(w0);
    alpha = sinw0 / (2.0 * quality_factor);
    a0 = (A + 1.0) + (A - 1.0) * cosw0 + 2.0 * sqrt(A) * alpha;

    filter->b0 = (float)(A * ((A + 1.0) - (A - 1.0) * cosw0 + 2.0 * sqrt(A) * alpha) / a0);
    filter->b1 = (float)(2.0 * A * ((A - 1.0) - (A + 1.0) * cosw0) / a0);
    filter->b2 = (float)(A * ((A + 1.0) - (A - 1.0) * cosw0 - 2.0 * sqrt(A) * alpha) / a0);
    filter->a1 = (float)(-2.0 * ((A - 1.0) + (A + 1.0) * cosw0) / a0);
    filter->a2 = (float)(((A + 1.0) + (A - 1.0) * cosw0 - 2.0 * sqrt(A) * alpha) / a0);
}

/* ハイシェルフフィルタの設定 */
void AE2BiquadFilter_SetHighShelf(
    struct AE2BiquadFilter *filter, float sampling_rate, float frequency, float quality_factor, float gain)
{
    double w0, alpha, A, sinw0, cosw0, a0;

    /* 引数チェック */
    assert(filter != NULL);
    assert(sampling_rate > 0.0f);
    assert(frequency > 0.0f);
    assert(quality_factor > 0.0f);
    assert(gain > 0.0f);

    A = sqrt(gain);
    w0 = 2.0 * AE2_PI * frequency / sampling_rate;
    sinw0 = sin(w0);
    cosw0 = cos(w0);
    alpha = sinw0 / (2.0 * quality_factor);
    a0 = (A + 1.0) - (A - 1.0) * cosw0 + 2.0 * sqrt(A) * alpha;

    filter->b0 = (float)(A * ((A + 1.0) + (A - 1.0) * cosw0 + 2.0 * sqrt(A) * alpha) / a0);
    filter->b1 = (float)(-2.0 * A * ((A - 1.0) + (A + 1.0) * cosw0) / a0);
    filter->b2 = (float)(A * ((A + 1.0) + (A - 1.0) * cosw0 - 2.0 * sqrt(A) * alpha) / a0);
    filter->a1 = (float)(2.0 * ((A - 1.0) - (A + 1.0) * cosw0) / a0);
    filter->a2 = (float)(((A + 1.0) - (A - 1.0) * cosw0 - 2.0 * sqrt(A) * alpha) / a0);
}

/* フィルタ内部状態リセット */
void AE2BiquadFilter_ClearBuffer(struct AE2BiquadFilter *filter)
{
    assert(filter != NULL);

    filter->x1 = filter->x2
        = filter->y1 = filter->y2 = 0.0f;
}

/* フィルタ適用 */
void AE2BiquadFilter_Process(
    struct AE2BiquadFilter *filter, float *data, uint32_t num_samples)
{
    uint32_t smpl;
    /* フィルタ係数をオート変数に受ける */
    const float a1 = filter->a1, a2 = filter->a2;
    const float b0 = filter->b0, b1 = filter->b1, b2 = filter->b2;

    assert(filter != NULL);
    assert(data != NULL);
    assert(num_samples > 0);

    smpl = 0;

    /* 4サンプル単位でフィルタ適用 */
    if (num_samples > 4) {
        float x0, x1, x2, x3, x4, x5;
        float y0, y1, y2, y3;

        /* オート変数初期化 */
        x4 = filter->x1; x5 = filter->x2;
        y0 = filter->y1; y1 = filter->y2;

        for (; smpl < num_samples - 4; smpl += 4) {
            float *pdata = &data[smpl];
            x3 = pdata[0]; x2 = pdata[1]; x1 = pdata[2]; x0 = pdata[3];

            y3 = b0 * x3 + b1 * x4 + b2 * x5 - a1 * y0 - a2 * y1;
            y2 = b0 * x2 + b1 * x3 + b2 * x4 - a1 * y3 - a2 * y0;
            y1 = b0 * x1 + b1 * x2 + b2 * x3 - a1 * y2 - a2 * y3;
            y0 = b0 * x0 + b1 * x1 + b2 * x2 - a1 * y1 - a2 * y2;

            pdata[0] = y3; pdata[1] = y2; pdata[2] = y1; pdata[3] = y0;
            x4 = x0; x5 = x1;
        }

        /* 最後の状態を記録 */
        filter->x1 = x0; filter->x2 = x1;
        filter->y1 = y0; filter->y2 = y1;
    }

    /* 1サンプル単位でフィルタ適用 */
    {
        float x0, x1, x2, y1, y2;

        /* オート変数初期化 */
        x1 = filter->x1; x2 = filter->x2;
        y1 = filter->y1; y2 = filter->y2;

        for (; smpl < num_samples; smpl++) {
            x0 = data[smpl];
            data[smpl] = b0 * x0 + b1 * x1 + b2 * x2 - a1 * y1 - a2 * y2;
            y2 = y1; y1 = data[smpl];
            x2 = x1; x1 = x0;
        }

        /* 最後の状態を記録 */
        filter->x1 = x1; filter->x2 = x2;
        filter->y1 = y1; filter->y2 = y2;
    }
}

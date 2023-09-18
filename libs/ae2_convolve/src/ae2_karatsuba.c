#include "ae2_karatsuba.h"

#include <assert.h>
#include <string.h>

#define AE2KARATSUBA_ALIGNMENT 16

/* 2値のうちの最大を取る */
#define MAX(x,y) (((x) > (y)) ? (x) : (y))
/* nの倍数切り上げ */
#define ROUNDUP(val, n) ((((val) + ((n) - 1)) / (n)) * (n))

struct AE2Karatsuba {
    float *coefficients; /* 畳み込み係数 */
    uint32_t num_coefficients; /* 畳み込み係数サイズ */
    float *input_buffer; /* 入力バッファ */
    float *output_buffer; /* 出力バッファ */
    float *work_buffer; /* 計算用ワークバッファ */
    int32_t output_buffer_pos; /* 出力バッファ参照位置 */
    uint32_t max_num_coefficients; /* 最大の畳み込み係数サイズ */
};

/* ワークサイズ計算 */
static int32_t AE2Karatsuba_CalculateWorkSize(const struct AE2ConvolveConfig *config);
/* インスタンス生成 */
static void* AE2Karatsuba_Create(const struct AE2ConvolveConfig *config, void *work, int32_t work_size);
/* インスタンス破棄 */
static void AE2Karatsuba_Destroy(void *obj);
/* 内部状態リセット */
static void AE2Karatsuba_Reset(void *obj);
/* 係数セット */
static void AE2Karatsuba_SetCoefficients(void *obj, const float *coefficients, uint32_t num_coefficients);
/* ワークサイズ計算 */
static void AE2Karatsuba_Convolve(void *obj, const float *input, float *output, uint32_t num_samples);
/* レイテンシーの取得 */
static int32_t AE2Karatsuba_GetLatencyNumSamples(void *obj);
/* ナイーブな畳込み */
/* z = a * b zはサイズ2n */
static void AE2Karatsuba_ConvolveNaive(const float *a, const float *b, float *z, uint32_t n);
/* カラツバ法による畳込み */
/* zはサイズ6n 先頭2nに結果が入る */
static void AE2Karatsuba_ConvolveKaratsuba(const float *a, const float *b, float *z, uint32_t n);
/* 2の冪乗に切り上げ */
static uint32_t AE2Karatsuba_Roundup2PoweredValue(uint32_t val);

/* インターフェース */
static const struct AE2ConvolveInterface st_karatsuba_convolve_if = {
    AE2Karatsuba_CalculateWorkSize,
    AE2Karatsuba_Create,
    AE2Karatsuba_Destroy,
    AE2Karatsuba_Reset,
    AE2Karatsuba_SetCoefficients,
    AE2Karatsuba_Convolve,
    AE2Karatsuba_GetLatencyNumSamples,
};

/* インターフェース取得 */
const struct AE2ConvolveInterface* AE2Karatsuba_GetInterface(void)
{
    return &st_karatsuba_convolve_if;
}

/* ワークサイズ計算 */
static int32_t AE2Karatsuba_CalculateWorkSize(const struct AE2ConvolveConfig* config)
{
    int32_t work_size;
    uint32_t max_num_block_samples;

    if (config == NULL) {
        return -1;
    }

    /* 最大処理サンプル単位 */
    max_num_block_samples = AE2Karatsuba_Roundup2PoweredValue(MAX(config->max_num_coefficients, config->max_num_input_samples));

    work_size = sizeof(struct AE2Karatsuba) + AE2KARATSUBA_ALIGNMENT;

    /* 係数1 + 入力バッファ1 + 出力バッファ1 + 計算バッファ6 */
    work_size += 9 * (sizeof(float) * max_num_block_samples + AE2KARATSUBA_ALIGNMENT);

    return work_size;
}

/* インスタンス生成 */
static void* AE2Karatsuba_Create(const struct AE2ConvolveConfig *config, void *work, int32_t work_size)
{
    uint8_t *work_ptr = (uint8_t *)work;
    struct AE2Karatsuba *conv;
    uint32_t max_num_block_samples;

    /* 引数チェック */
    if ((work == NULL) || (config == NULL)
            || (work_size < AE2Karatsuba_CalculateWorkSize(config))) {
        return NULL;
    }

    /* 最大処理サンプル単位 */
    max_num_block_samples = AE2Karatsuba_Roundup2PoweredValue(MAX(config->max_num_coefficients, config->max_num_input_samples));

    /* 構造体を配置 */
    work_ptr = (uint8_t *)ROUNDUP((uintptr_t)work_ptr, AE2KARATSUBA_ALIGNMENT);
    conv = (struct AE2Karatsuba *)work_ptr;
    conv->num_coefficients = 0;
    conv->output_buffer_pos = 0;
    conv->max_num_coefficients = max_num_block_samples;
    work_ptr += sizeof(struct AE2Karatsuba);

    /* 係数領域の割り当て */
    work_ptr = (uint8_t *)ROUNDUP((uintptr_t)work_ptr, AE2KARATSUBA_ALIGNMENT);
    conv->coefficients = (float *)work_ptr;
    work_ptr += sizeof(float) * max_num_block_samples;

    /* 入力バッファの割り当て */
    work_ptr = (uint8_t *)ROUNDUP((uintptr_t)work_ptr, AE2KARATSUBA_ALIGNMENT);
    conv->input_buffer = (float *)work_ptr;
    work_ptr += sizeof(float) * max_num_block_samples;

    /* 出力バッファの割り当て */
    work_ptr = (uint8_t *)ROUNDUP((uintptr_t)work_ptr, AE2KARATSUBA_ALIGNMENT);
    conv->output_buffer = (float *)work_ptr;
    work_ptr += sizeof(float) * max_num_block_samples;

    /* 計算用ワークバッファの割り当て */
    work_ptr = (uint8_t *)ROUNDUP((uintptr_t)work_ptr, AE2KARATSUBA_ALIGNMENT);
    conv->work_buffer = (float *)work_ptr;
    work_ptr += 6 * sizeof(float) * max_num_block_samples;

    /* バッファをリセット */
    AE2Karatsuba_Reset(conv);

    return conv;
}

/* インスタンス破棄 */
static void AE2Karatsuba_Destroy(void *obj)
{
    /* 特に何もしない */
    if (obj != NULL) {
        return;
    }
}

/* 係数セット */
static void AE2Karatsuba_SetCoefficients(void *obj, const float *coefficients, uint32_t num_coefficients)
{
    uint32_t i;
    struct AE2Karatsuba* conv = (struct AE2Karatsuba *)obj;

    /* 引数チェック */
    assert(obj != NULL);

    /* 係数サイズチェック */
    assert(num_coefficients <= conv->max_num_coefficients);

    /* 係数を単純コピー */
    memcpy(conv->coefficients, coefficients, sizeof(float) * num_coefficients);

    /* 係数サイズは2の冪乗に切り上げておく */
    conv->num_coefficients = AE2Karatsuba_Roundup2PoweredValue(num_coefficients);

    /* 係数末尾は0埋め */
    for (i = num_coefficients; i < conv->max_num_coefficients; i++) {
        conv->coefficients[i] = 0.0f;
    }

    /* 内部バッファリセット */
    AE2Karatsuba_Reset(obj);
}

/* 畳み込み計算 */
static void AE2Karatsuba_Convolve(void *obj, const float *input, float *output, uint32_t num_samples)
{
    uint32_t smpl, i, conv_size;
    struct AE2Karatsuba* conv = (struct AE2Karatsuba *)obj;

    /* 引数チェック */
    assert((obj != NULL) && (input != NULL) && (output != NULL));

    /* 畳み込みサイズの確定: 必ず2の冪乗, かつ係数分畳み込むように十分なサイズを選ぶ */
    conv_size = AE2Karatsuba_Roundup2PoweredValue(MAX(num_samples, conv->num_coefficients));

    /* 入力バッファにデータを入力 */
    memcpy(conv->input_buffer, input, sizeof(float) * num_samples);
    /* 入力サンプル以降は0埋め */
    for (smpl = num_samples; smpl < conv_size; smpl++) {
        conv->input_buffer[smpl] = 0.0f;
    }

    /* 畳み込み計算 */
    AE2Karatsuba_ConvolveKaratsuba(conv->input_buffer,
            conv->coefficients, conv->work_buffer, conv_size);

    /* 先頭のnum_samplesは前回の余りを加算してそのまま出力 */
    for (smpl = 0; smpl < num_samples; smpl++) {
        output[smpl] = conv->output_buffer[smpl] + conv->work_buffer[smpl];
    }

    /* 次回処理のために余り（FIRフィルタの遅延）分出力バッファを更新 */
    /* 加算 */
    i = 0;
    for (; smpl < conv_size; smpl++) {
        conv->output_buffer[i++] = conv->output_buffer[smpl] + conv->work_buffer[smpl];
    }
    /* 上書き */
    for (; smpl < conv_size + num_samples; smpl++) {
        conv->output_buffer[i++] = conv->work_buffer[smpl];
    }
}

/* 内部状態リセット */
static void AE2Karatsuba_Reset(void *obj)
{
    uint32_t i;
    struct AE2Karatsuba *conv = (struct AE2Karatsuba *)obj;

    /* 入力バッファのクリア */
    for (i = 0; i < conv->max_num_coefficients; i++) {
        conv->input_buffer[i] = 0.0f;
    }

    /* 出力バッファのクリア */
    for (i = 0; i < conv->max_num_coefficients; i++) {
        conv->output_buffer[i] = 0.0f;
    }

    /* 計算用ワークバッファのクリア */
    for (i = 0; i < 6 * conv->max_num_coefficients; i++) {
        conv->work_buffer[i] = 0.0f;
    }

    /* バッファ参照位置のクリア */
    conv->output_buffer_pos = 0;
}

/* レイテンシーの取得 */
static int32_t AE2Karatsuba_GetLatencyNumSamples(void *obj)
{
    /* レイテンシー0 */
    (void)obj;
    return 0;
}

/* 素朴な直線畳込み */
/* z = a * b zはサイズ2n */
static void AE2Karatsuba_ConvolveNaive(const float *a, const float *b, float *z, uint32_t n)
{
    uint32_t i, j;

    /* 初期化 */
    for (i = 0; i < (n << 1); i++) {
        z[i] = 0.0f;
    }

    /* 畳み込み */
    for (j = 0; j < n; j++) {
        for (i = 0; i < n; i++) {
            z[j + i] += a[i] * b[j];
        }
    }
}

/* カラツバ法による畳込み */
/* zはサイズ6n 先頭2nに結果が入る */
static void AE2Karatsuba_ConvolveKaratsuba(const float *a, const float *b, float *z, uint32_t n)
{
    uint32_t i;
    const uint32_t  n2 = n >> 1;
    const float     *a0 = &a[0];        /* 被乗数/右側配列ポインタ        */
    const float     *a1 = &a[n2];       /* 被乗数/左側配列ポインタ        */
    const float     *b0 = &b[0];        /* 乗数  /右側配列ポインタ        */
    const float     *b1 = &b[n2];       /* 乗数  /左側配列ポインタ        */
    float     *x1 = &z[n * 0];          /* x1 (= a0 * b0) 用配列ポインタ  */
    float     *x2 = &z[n * 1];          /* x2 (= a1 * b1) 用配列ポインタ  */
    float     *x3 = &z[n * 2];          /* x3 (= v * w)   用配列ポインタ  */
    float     *v  = &z[n * 5];          /* v  (= a1 + a0) 用配列ポインタ  */
    float     *w  = &z[n * 5 + n2];     /* w  (= b1 + b0) 用配列ポインタ  */

    /* サイズが8以下の場合は通常の畳込みを行う */
    if (n <= 8) {
        assert(n == 8);
        AE2Karatsuba_ConvolveNaive(a, b, z, 8);
        return;
    }

    /* v = a1 + a0, w = b1 + b0 */
    for(i = 0; i < n2; i++) {
        v[i] = a1[i] + a0[i];
        w[i] = b1[i] + b0[i];
    }

    /* x1 = a0 * b0 */
    AE2Karatsuba_ConvolveKaratsuba(a0, b0, x1, n2);

    /* x2 = a1 * b1 */
    AE2Karatsuba_ConvolveKaratsuba(a1, b1, x2, n2);

    /* x3 = (a1 + a0) * (b1 + b0) */
    AE2Karatsuba_ConvolveKaratsuba(v,  w,  x3, n2);

    /* x3 -= x1 + x2 */
    for(i = 0; i < n; i++) {
        x3[i] -= (x1[i] + x2[i]);
    }

    /* z = x2 * R^2 + (x3 - x1 - x2) * R + x1 */
    /*      ( x1, x2 は既に所定の位置にセットされているので、x3 のみ加算 ) */
    for(i = 0; i < n; i++) {
        z[i + n2] += x3[i];
    }

}

/* 2の冪乗に切り上げ */
/* ハッカーのたのしみより引用 */
static uint32_t AE2Karatsuba_Roundup2PoweredValue(uint32_t val)
{
    val--;
    val |= val >> 1;
    val |= val >> 2;
    val |= val >> 4;
    val |= val >> 8;
    val |= val >> 16;
    val++;

    return val;
}

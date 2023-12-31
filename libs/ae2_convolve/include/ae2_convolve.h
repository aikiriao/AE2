#ifndef AE2CONVOLVE_H_INCLUDED
#define AE2CONVOLVE_H_INCLUDED

#include <stdint.h>

/* 初期化コンフィグ */
struct AE2ConvolveConfig {
    uint32_t max_num_coefficients; /* 最大係数数 */
    uint32_t max_num_input_samples; /* 最大入力サンプル数 */
};

/* 畳み込みインターフェース */
struct AE2ConvolveInterface {
    /* ワークサイズ計算 */
    int32_t (*CalculateWorkSize)(const struct AE2ConvolveConfig *config);
    /* インスタンス作成 */
    void* (*Create)(const struct AE2ConvolveConfig *config, void *work, int32_t work_size);
    /* インスタンス破棄 */
    void (*Destroy)(void *obj);
    /* 内部状態リセット */
    void (*Reset)(void *obj);
    /* 畳み込み係数セット */
    void (*SetCoefficients)(void *obj, const float *coefficients, uint32_t num_coefficients);
    /* 畳み込み演算実行 */
    void (*Convolve)(void *obj, const float *input, float *output, uint32_t num_samples);
    /* レイテンシーの取得 */
    int32_t (*GetLatencyNumSamples)(void *obj);
};

#endif /* AE2CONVOLVE_H_INCLUDED */

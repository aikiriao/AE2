#include "ae2_window_function.h"

#include <math.h>
#include <assert.h>
#include <stddef.h>

/* 円周率 */
#define AE2_PI 3.14159265358979323846

/*!
* @brief ハン窓
*/
static inline float AE2WindowFunction_HannWindow(uint32_t x, uint32_t window_size)
{
    return (float)(0.5 * (1.0 - cos((2.0 * AE2_PI * x) / (window_size - 1))));
}

/*!
* @brief ハミング窓
*/
static inline float AE2WindowFunction_HammingWindow(uint32_t x, uint32_t window_size)
{
    return (float)(0.54 - 0.46 * cos((2.0 * AE2_PI * x) / (window_size - 1)));
}

/*!
* @brief ブラックマン窓
*/
static inline float AE2WindowFunction_BlackmanWindow(uint32_t x, uint32_t window_size)
{
    return (float)(0.42 - 0.5 * cos((2.0 * AE2_PI * x) / (window_size - 1)) + 0.08 * cos((4.0 * AE2_PI * x) / (window_size - 1)));
}

/* 矩形窓の適用 */
static void AE2WindowFunction_ApplyRectangularWindow(float *data, uint32_t num_samples)
{
    assert(data != NULL);
    assert(num_samples > 0);

    /* 何もしない */
}

/* ハン窓の適用 */
static void AE2WindowFunction_ApplyHannWindow(float *data, uint32_t num_samples)
{
    uint32_t smpl;
    
    assert(data != NULL);
    assert(num_samples > 0);
    
    for (smpl = 0; smpl < num_samples; smpl++) {
            data[smpl] *= AE2WindowFunction_HannWindow(smpl, num_samples);
    }
}

/* ハミング窓の適用 */
static void AE2WindowFunction_ApplyHammingWindow(float *data, uint32_t num_samples)
{
    uint32_t smpl;
    
    assert(data != NULL);
    assert(num_samples > 0);
    
    for (smpl = 0; smpl < num_samples; smpl++) {
            data[smpl] *= AE2WindowFunction_HammingWindow(smpl, num_samples);
    }
}

/* ブラックマン窓の作成 */
static void AE2WindowFunction_ApplyBlackmanWindow(float *data, uint32_t num_samples)
{
    uint32_t smpl;
    
    assert(data != NULL);
    assert(num_samples > 0);
    
    for (smpl = 0; smpl < num_samples; smpl++) {
            data[smpl] *= AE2WindowFunction_BlackmanWindow(smpl, num_samples);
    }
}


/* 矩形窓の作成 */
static void AE2WindowFunction_MakeRectangularWindow(float *window, uint32_t window_size)
{
    uint32_t x;

    assert(window != NULL);
    assert(window_size > 0);

    for (x = 0; x < window_size; x++) {
        window[x] = 1.0f;
    }
}

/* 窓の作成 */
void AE2WindowFunction_MakeWindow(AE2WindowFunctionType type, float *window, uint32_t window_size)
{
    assert(window != NULL);
    assert(window_size > 0);

    switch (type) {
    case AE2WINDOWFUNCTION_RECTANGULAR:
        AE2WindowFunction_MakeRectangularWindow(window, window_size);
        break;
    case AE2WINDOWFUNCTION_HANN:
        AE2WindowFunction_MakeRectangularWindow(window, window_size);
        AE2WindowFunction_ApplyHannWindow(window, window_size);
        break;
    case AE2WINDOWFUNCTION_HAMMING:
        AE2WindowFunction_MakeRectangularWindow(window, window_size);
        AE2WindowFunction_ApplyHammingWindow(window, window_size);
        break;
    case AE2WINDOWFUNCTION_BLACKMAN:
        AE2WindowFunction_MakeRectangularWindow(window, window_size);
        AE2WindowFunction_ApplyBlackmanWindow(window, window_size);
        break;
    default:
        assert(0);
    }
}

/* 窓の適用 */
void AE2WindowFunction_ApplyWindow(AE2WindowFunctionType type, float *data, uint32_t num_samples)
{
    assert(data != NULL);
    assert(num_samples > 0);

    switch (type) {
    case AE2WINDOWFUNCTION_RECTANGULAR:
        AE2WindowFunction_ApplyRectangularWindow(data, num_samples);
        break;
    case AE2WINDOWFUNCTION_HANN:
        AE2WindowFunction_ApplyHannWindow(data, num_samples);
        break;
    case AE2WINDOWFUNCTION_HAMMING:
        AE2WindowFunction_ApplyHammingWindow(data, num_samples);
        break;
    case AE2WINDOWFUNCTION_BLACKMAN:
        AE2WindowFunction_ApplyBlackmanWindow(data, num_samples);
        break;
    default:
        assert(0);
    }
}

#include <stdlib.h>
#include <string.h>

#include <gtest/gtest.h>

/* テスト対象のモジュール */
extern "C" {
#include "../../libs/ae2_iir_filter/src/ae2_biquad_filter.c"
}

/* 周波数特性計測のためFFT使用 */
#include "ae2_fft.h"

/* スペクトル計算 */
static void AE2BiquadFilterTest_ComputeSpectrum(float *data, uint32_t size)
{
    uint32_t bin;
    float *fftwork = (float *)malloc(size * sizeof(float));

    AE2FFT_RealFFT(size, -1, data, fftwork);

    /* 絶対値計算 */
    data[0] = fabs(data[0]); /* 直流成分 */
    for (bin = 1; bin < size / 2; bin++) {
        data[bin]
            = sqrtf(AE2FFTCOMPLEX_REAL(data, bin) * AE2FFTCOMPLEX_REAL(data, bin)
                + AE2FFTCOMPLEX_IMAG(data, bin) * AE2FFTCOMPLEX_IMAG(data, bin));
    }

    free(fftwork);
}

/* 位相スペクトル計算 */
static void AE2BiquadFilterTest_ComputePhaseSpectrum(float *data, uint32_t size)
{
    uint32_t bin;
    float *fftwork = (float *)malloc(size * sizeof(float));

    AE2FFT_RealFFT(size, -1, data, fftwork);

    /* 位相計算 */
    data[0] = 0.0f; /* 直流成分 */
    for (bin = 1; bin < size / 2; bin++) {
        data[bin] = atan2f(AE2FFTCOMPLEX_IMAG(data, bin), AE2FFTCOMPLEX_REAL(data, bin));
    }

    free(fftwork);
}

/* フィルタデザインテスト */
TEST(AE2BiquadFilterTest, FilterDesignTest)
{
#define TEST_NUM_SAMPLES 256

    /* テストケース */
    static const struct FilterTestCase {
        int frequency; /* 周波数（注目するビン） */
        float quality_factor; /* Q値 */
        float gain; /* ゲイン */
    } filter_test_case[] = {
        {       TEST_NUM_SAMPLES / 8, (float)(1.0 / sqrt(2.0)), 1.0 },
        {       TEST_NUM_SAMPLES / 8,                      1.0, 1.0 },
        {       TEST_NUM_SAMPLES / 8,                      4.0, 1.0 },
        {       TEST_NUM_SAMPLES / 8,                     16.0, 1.0 },
        {       TEST_NUM_SAMPLES / 4, (float)(1.0 / sqrt(2.0)), 1.0 },
        {       TEST_NUM_SAMPLES / 4,                      1.0, 1.0 },
        {       TEST_NUM_SAMPLES / 4,                      4.0, 1.0 },
        {       TEST_NUM_SAMPLES / 4,                     16.0, 1.0 },
        { (3 * TEST_NUM_SAMPLES) / 8, (float)(1.0 / sqrt(2.0)), 1.0 },
        { (3 * TEST_NUM_SAMPLES) / 8,                      1.0, 1.0 },
        { (3 * TEST_NUM_SAMPLES) / 8,                      4.0, 1.0 },
        { (3 * TEST_NUM_SAMPLES) / 8,                     16.0, 1.0 },
        {       TEST_NUM_SAMPLES / 8, (float)(1.0 / sqrt(2.0)), 0.1 },
        {       TEST_NUM_SAMPLES / 8,                      1.0, 0.1 },
        {       TEST_NUM_SAMPLES / 8,                      4.0, 0.1 },
        {       TEST_NUM_SAMPLES / 8,                     16.0, 0.1 },
        {       TEST_NUM_SAMPLES / 4, (float)(1.0 / sqrt(2.0)), 0.1 },
        {       TEST_NUM_SAMPLES / 4,                      1.0, 0.1 },
        {       TEST_NUM_SAMPLES / 4,                      4.0, 0.1 },
        {       TEST_NUM_SAMPLES / 4,                     16.0, 0.1 },
        { (3 * TEST_NUM_SAMPLES) / 8, (float)(1.0 / sqrt(2.0)), 0.1 },
        { (3 * TEST_NUM_SAMPLES) / 8,                      1.0, 0.1 },
        { (3 * TEST_NUM_SAMPLES) / 8,                      4.0, 0.1 },
        { (3 * TEST_NUM_SAMPLES) / 8,                     16.0, 0.1 },
        {       TEST_NUM_SAMPLES / 8, (float)(1.0 / sqrt(2.0)), 5.0 },
        {       TEST_NUM_SAMPLES / 8,                      1.0, 5.0 },
        {       TEST_NUM_SAMPLES / 8,                      4.0, 5.0 },
        {       TEST_NUM_SAMPLES / 8,                     16.0, 5.0 },
        {       TEST_NUM_SAMPLES / 4, (float)(1.0 / sqrt(2.0)), 5.0 },
        {       TEST_NUM_SAMPLES / 4,                      1.0, 5.0 },
        {       TEST_NUM_SAMPLES / 4,                      4.0, 5.0 },
        {       TEST_NUM_SAMPLES / 4,                     16.0, 5.0 },
        { (3 * TEST_NUM_SAMPLES) / 8, (float)(1.0 / sqrt(2.0)), 5.0 },
        { (3 * TEST_NUM_SAMPLES) / 8,                      1.0, 5.0 },
        { (3 * TEST_NUM_SAMPLES) / 8,                      4.0, 5.0 },
        { (3 * TEST_NUM_SAMPLES) / 8,                     16.0, 5.0 },
    };
    static const uint32_t num_test_cases = sizeof(filter_test_case) / sizeof(filter_test_case[0]);

    /* ローパスフィルタのテスト */
    {
        float data[TEST_NUM_SAMPLES];
        struct AE2BiquadFilter filter;
        uint32_t i, j;

        for (i = 0; i < num_test_cases; i++) {
            const struct FilterTestCase *ptest = &filter_test_case[i];

            ASSERT_TRUE(ptest->frequency < (TEST_NUM_SAMPLES / 2));

            /* フィルタ作成 */
            AE2BiquadFilter_SetLowPass(&filter, TEST_NUM_SAMPLES, ptest->frequency, ptest->quality_factor, ptest->gain);

            /* 単位インパルス生成 */
            data[0] = 1.0f;
            for (j = 1; j < TEST_NUM_SAMPLES; j++) {
                data[j] = 0.0f;
            }

            /* フィルタ適用 */
            AE2BiquadFilter_ClearBuffer(&filter);
            AE2BiquadFilter_Process(&filter, data, TEST_NUM_SAMPLES);

            /* スペクトル計算 */
            AE2BiquadFilterTest_ComputeSpectrum(data, TEST_NUM_SAMPLES);

            /* 通過域でgain, 遷移域でQ * gain，遮断域で0.0に近いことを期待 */
            EXPECT_NEAR(ptest->gain, data[0], ptest->gain * 1e-2);
            EXPECT_NEAR(ptest->quality_factor * ptest->gain, data[ptest->frequency], ptest->quality_factor * ptest->gain * 1e-2);
            EXPECT_NEAR(0.0, data[TEST_NUM_SAMPLES / 2 - 1], 5e-2);
        }
    }

    /* ハイパスフィルタのテスト */
    {
        float data[TEST_NUM_SAMPLES];
        struct AE2BiquadFilter filter;
        uint32_t i, j;

        for (i = 0; i < num_test_cases; i++) {
            const struct FilterTestCase *ptest = &filter_test_case[i];

            ASSERT_TRUE(ptest->frequency < (TEST_NUM_SAMPLES / 2));

            /* フィルタ作成 */
            AE2BiquadFilter_SetHighPass(&filter, TEST_NUM_SAMPLES, ptest->frequency, ptest->quality_factor, ptest->gain);

            /* 単位インパルス生成 */
            data[0] = 1.0f;
            for (j = 1; j < TEST_NUM_SAMPLES; j++) {
                data[j] = 0.0f;
            }

            /* フィルタ適用 */
            AE2BiquadFilter_ClearBuffer(&filter);
            AE2BiquadFilter_Process(&filter, data, TEST_NUM_SAMPLES);

            /* スペクトル計算 */
            AE2BiquadFilterTest_ComputeSpectrum(data, TEST_NUM_SAMPLES);

            /* 通過域でgain, 遷移域でQ * gain，遮断域で0.0に近いことを期待 */
            EXPECT_NEAR(0.0, data[0], 5e-2);
            EXPECT_NEAR(ptest->quality_factor * ptest->gain, data[ptest->frequency], ptest->quality_factor * ptest->gain * 1e-2);
            EXPECT_NEAR(ptest->gain, data[TEST_NUM_SAMPLES / 2 - 1], ptest->gain * 5e-2);
        }
    }

    /* バンドパスフィルタのテスト */
    {
        float data[TEST_NUM_SAMPLES];
        struct AE2BiquadFilter filter;
        uint32_t i, j;

        for (i = 0; i < num_test_cases; i++) {
            const struct FilterTestCase *ptest = &filter_test_case[i];

            ASSERT_TRUE(ptest->frequency < (TEST_NUM_SAMPLES / 2));

            /* フィルタ作成 */
            AE2BiquadFilter_SetBandPass(&filter, TEST_NUM_SAMPLES, ptest->frequency, ptest->quality_factor, ptest->gain);

            /* 単位インパルス生成 */
            data[0] = 1.0f;
            for (j = 1; j < TEST_NUM_SAMPLES; j++) {
                data[j] = 0.0f;
            }

            /* フィルタ適用 */
            AE2BiquadFilter_ClearBuffer(&filter);
            AE2BiquadFilter_Process(&filter, data, TEST_NUM_SAMPLES);

            /* スペクトル計算 */
            AE2BiquadFilterTest_ComputeSpectrum(data, TEST_NUM_SAMPLES);

            /* 注目する周波数でgainになることを期待 */
            EXPECT_NEAR(ptest->gain, data[ptest->frequency], ptest->gain * 1e-2);
        }
    }

    /* バンドエリミネートフィルタのテスト */
    {
        float data[TEST_NUM_SAMPLES];
        struct AE2BiquadFilter filter;
        uint32_t i, j;

        for (i = 0; i < num_test_cases; i++) {
            const struct FilterTestCase *ptest = &filter_test_case[i];

            ASSERT_TRUE(ptest->frequency < (TEST_NUM_SAMPLES / 2));

            /* フィルタ作成 */
            AE2BiquadFilter_SetBandEliminate(&filter, TEST_NUM_SAMPLES, ptest->frequency, ptest->quality_factor, ptest->gain);

            /* 単位インパルス生成 */
            data[0] = 1.0f;
            for (j = 1; j < TEST_NUM_SAMPLES; j++) {
                data[j] = 0.0f;
            }

            /* フィルタ適用 */
            AE2BiquadFilter_ClearBuffer(&filter);
            AE2BiquadFilter_Process(&filter, data, TEST_NUM_SAMPLES);

            /* スペクトル計算 */
            AE2BiquadFilterTest_ComputeSpectrum(data, TEST_NUM_SAMPLES);

            /* 注目する周波数で0.0になることを期待 */
            EXPECT_NEAR(ptest->gain, data[0], 5e-2);
            EXPECT_NEAR(0.0, data[ptest->frequency], 5e-2);
            EXPECT_NEAR(ptest->gain, data[TEST_NUM_SAMPLES / 2 - 1], 5e-2);
        }
    }

    /* オールパスフィルタのテスト */
    {
        float data[TEST_NUM_SAMPLES], phase[TEST_NUM_SAMPLES];
        struct AE2BiquadFilter filter;
        uint32_t i, j;

        for (i = 0; i < num_test_cases; i++) {
            const struct FilterTestCase *ptest = &filter_test_case[i];

            ASSERT_TRUE(ptest->frequency < (TEST_NUM_SAMPLES / 2));

            /* フィルタ作成 */
            AE2BiquadFilter_SetAllPass(&filter, TEST_NUM_SAMPLES, ptest->frequency, ptest->quality_factor, ptest->gain);

            /* 単位インパルス生成 */
            data[0] = 1.0f;
            for (j = 1; j < TEST_NUM_SAMPLES; j++) {
                data[j] = 0.0f;
            }

            /* フィルタ適用 */
            AE2BiquadFilter_ClearBuffer(&filter);
            AE2BiquadFilter_Process(&filter, data, TEST_NUM_SAMPLES);

            /* スペクトル計算 */
            memcpy(phase, data, TEST_NUM_SAMPLES * sizeof(float));
            AE2BiquadFilterTest_ComputeSpectrum(data, TEST_NUM_SAMPLES);
            AE2BiquadFilterTest_ComputePhaseSpectrum(phase, TEST_NUM_SAMPLES);

            /* 振幅はすべてgainに近いことを期待 */
            {
                int32_t is_ok = 1;
                for (j = 0; j < TEST_NUM_SAMPLES / 2; j++) {
                    if (fabs(data[j] - ptest->gain) > ptest->gain * 1e-2) {
                        is_ok = 0;
                        break;
                    }
                }
                EXPECT_EQ(1, is_ok);
            }

            /* 注目する周波数で位相反転しているはず */
            EXPECT_NEAR(AE2_PI, fabs(phase[ptest->frequency]), 1e-3);
        }
    }

    /* ローシェルフフィルタのテスト */
    {
        float data[TEST_NUM_SAMPLES];
        struct AE2BiquadFilter filter;
        uint32_t i, j;

        for (i = 0; i < num_test_cases; i++) {
            const struct FilterTestCase *ptest = &filter_test_case[i];

            ASSERT_TRUE(ptest->frequency < (TEST_NUM_SAMPLES / 2));

            /* フィルタ作成 */
            AE2BiquadFilter_SetLowShelf(&filter, TEST_NUM_SAMPLES, ptest->frequency, ptest->quality_factor, ptest->gain);

            /* 単位インパルス生成 */
            data[0] = 1.0f;
            for (j = 1; j < TEST_NUM_SAMPLES; j++) {
                data[j] = 0.0f;
            }

            /* フィルタ適用 */
            AE2BiquadFilter_ClearBuffer(&filter);
            AE2BiquadFilter_Process(&filter, data, TEST_NUM_SAMPLES);

            /* スペクトル計算 */
            AE2BiquadFilterTest_ComputeSpectrum(data, TEST_NUM_SAMPLES);

            /* 通過域でgain, 遷移域でgain，遮断域で1.0に近いことを期待 */
            EXPECT_NEAR(ptest->gain, data[0], ptest->gain * 5e-2);
            EXPECT_NEAR(1.0, data[TEST_NUM_SAMPLES / 2 - 1], 5e-2);
        }
    }

    /* ハイシェルフフィルタのテスト */
    {
        float data[TEST_NUM_SAMPLES];
        struct AE2BiquadFilter filter;
        uint32_t i, j;

        for (i = 0; i < num_test_cases; i++) {
            const struct FilterTestCase *ptest = &filter_test_case[i];

            ASSERT_TRUE(ptest->frequency < (TEST_NUM_SAMPLES / 2));

            /* フィルタ作成 */
            AE2BiquadFilter_SetHighShelf(&filter, TEST_NUM_SAMPLES, ptest->frequency, ptest->quality_factor, ptest->gain);

            /* 単位インパルス生成 */
            data[0] = 1.0f;
            for (j = 1; j < TEST_NUM_SAMPLES; j++) {
                data[j] = 0.0f;
            }

            /* フィルタ適用 */
            AE2BiquadFilter_ClearBuffer(&filter);
            AE2BiquadFilter_Process(&filter, data, TEST_NUM_SAMPLES);

            /* スペクトル計算 */
            AE2BiquadFilterTest_ComputeSpectrum(data, TEST_NUM_SAMPLES);

            /* 通過域でgain, 遷移域でgain，遮断域で1.0に近いことを期待 */
            EXPECT_NEAR(1.0, data[0], 5e-2);
            EXPECT_NEAR(ptest->gain, data[TEST_NUM_SAMPLES / 2 - 1], ptest->gain * 5e-2);
        }
    }

#undef TEST_NUM_SAMPLES
}

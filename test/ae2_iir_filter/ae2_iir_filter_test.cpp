#include <stdlib.h>
#include <string.h>

#include <gtest/gtest.h>

/* テスト対象のモジュール */
extern "C" {
#include "../../libs/ae2_iir_filter/src/ae2_iir_filter.c"
}

/* 周波数特性計測のためFFT使用 */
#include "ae2_fft.h"

/* デコードハンドル作成破棄テスト */
TEST(AE2IIRFilterTest, CreateDestroyTest)
{
    /* ワークサイズ計算テスト */
    {
        int32_t work_size;

        /* 簡単な成功例 */
        work_size = AE2IIRFilter_CalculateWorkSize(1);
        EXPECT_TRUE(work_size >= (int32_t)sizeof(struct AE2IIRFilter));

        /* 不正な次数 */
        work_size = AE2IIRFilter_CalculateWorkSize(0);
        EXPECT_TRUE(work_size <  0);
    }

    /* ワーク領域渡しによるハンドル作成（成功例） */
    {
        void *work;
        int32_t work_size;
        struct AE2IIRFilter *filter;

        work_size = AE2IIRFilter_CalculateWorkSize(1);
        work = malloc(work_size);

        filter = AE2IIRFilter_Create(1, work, work_size);
        EXPECT_TRUE(filter != NULL);

        AE2IIRFilter_Destroy(filter);
        free(work);
    }

    /* ワーク領域渡しによるハンドル作成（失敗ケース） */
    {
        void *work;
        int32_t work_size;
        struct AE2IIRFilter *decoder;

        work_size = AE2IIRFilter_CalculateWorkSize(1);
        work = malloc(work_size);

        /* 引数が不正 */
        decoder = AE2IIRFilter_Create(1, NULL, work_size);
        EXPECT_TRUE(decoder == NULL);
        decoder = AE2IIRFilter_Create(1, work, 0);
        EXPECT_TRUE(decoder == NULL);

        /* ワークサイズ不足 */
        decoder = AE2IIRFilter_Create(1, work, work_size - 1);
        EXPECT_TRUE(decoder == NULL);

        free(work);
    }
}

/* スペクトル計算 */
static void AE2IIRFilterTest_ComputeSpectrum(float *data, uint32_t size)
{
    uint32_t bin;
    float *fftwork = (float *)malloc(size * sizeof(float));

    AE2FFT_RealFFT(size, -1, data, fftwork);

    /* 絶対値計算 */
    data[0] = fabs(data[0]); /* 直流成分 */
    for (bin = 1; bin < size / 2; bin++) {
        data[bin]
            = sqrt(AE2FFTCOMPLEX_REAL(data, bin) * AE2FFTCOMPLEX_REAL(data, bin)
                + AE2FFTCOMPLEX_IMAG(data, bin) * AE2FFTCOMPLEX_IMAG(data, bin));
    }

    free(fftwork);
}

/* 位相スペクトル計算 */
static void AE2IIRFilterTest_ComputePhaseSpectrum(float *data, uint32_t size)
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

/* バターワースフィルタデザインテスト */
TEST(AE2IIRFilterTest, ButterworthFilterDesignTest)
{
#define TEST_NUM_SAMPLES 1024

    /* テストケース */
    static const struct ButterworthFilterTestCase {
        int32_t order; /* 次数 */
        int32_t frequency; /* 周波数（注目するビン） */
    } filter_test_case[] = {
        { 50,       TEST_NUM_SAMPLES / 8 },
        {  2,       TEST_NUM_SAMPLES / 8 },
        {  2,       TEST_NUM_SAMPLES / 8 },
        {  2,       TEST_NUM_SAMPLES / 8 },
        {  2,       TEST_NUM_SAMPLES / 4 },
        {  2,       TEST_NUM_SAMPLES / 4 },
        {  2,       TEST_NUM_SAMPLES / 4 },
        {  2,       TEST_NUM_SAMPLES / 4 },
        {  2, (3 * TEST_NUM_SAMPLES) / 8 },
        {  2, (3 * TEST_NUM_SAMPLES) / 8 },
        {  2, (3 * TEST_NUM_SAMPLES) / 8 },
        {  2, (3 * TEST_NUM_SAMPLES) / 8 },
        {  3,       TEST_NUM_SAMPLES / 8 },
        {  3,       TEST_NUM_SAMPLES / 8 },
        {  3,       TEST_NUM_SAMPLES / 8 },
        {  3,       TEST_NUM_SAMPLES / 8 },
        {  3,       TEST_NUM_SAMPLES / 4 },
        {  3,       TEST_NUM_SAMPLES / 4 },
        {  3,       TEST_NUM_SAMPLES / 4 },
        {  3,       TEST_NUM_SAMPLES / 4 },
        {  3, (3 * TEST_NUM_SAMPLES) / 8 },
        {  3, (3 * TEST_NUM_SAMPLES) / 8 },
        {  3, (3 * TEST_NUM_SAMPLES) / 8 },
        {  3, (3 * TEST_NUM_SAMPLES) / 8 },
        { 50,       TEST_NUM_SAMPLES / 8 },
        { 50,       TEST_NUM_SAMPLES / 8 },
        { 50,       TEST_NUM_SAMPLES / 8 },
        { 50,       TEST_NUM_SAMPLES / 8 },
        { 50,       TEST_NUM_SAMPLES / 4 },
        { 50,       TEST_NUM_SAMPLES / 4 },
        { 50,       TEST_NUM_SAMPLES / 4 },
        { 50,       TEST_NUM_SAMPLES / 4 },
        { 50, (3 * TEST_NUM_SAMPLES) / 8 },
        { 50, (3 * TEST_NUM_SAMPLES) / 8 },
        { 50, (3 * TEST_NUM_SAMPLES) / 8 },
        { 50, (3 * TEST_NUM_SAMPLES) / 8 },
        { 51,       TEST_NUM_SAMPLES / 8 },
        { 51,       TEST_NUM_SAMPLES / 8 },
        { 51,       TEST_NUM_SAMPLES / 8 },
        { 51,       TEST_NUM_SAMPLES / 8 },
        { 51,       TEST_NUM_SAMPLES / 4 },
        { 51,       TEST_NUM_SAMPLES / 4 },
        { 51,       TEST_NUM_SAMPLES / 4 },
        { 51,       TEST_NUM_SAMPLES / 4 },
        { 51, (3 * TEST_NUM_SAMPLES) / 8 },
        { 51, (3 * TEST_NUM_SAMPLES) / 8 },
        { 51, (3 * TEST_NUM_SAMPLES) / 8 },
        { 51, (3 * TEST_NUM_SAMPLES) / 8 },
    };
    static const uint32_t num_test_cases = sizeof(filter_test_case) / sizeof(filter_test_case[0]);

    /* ローパスフィルタのテスト */
    {
        float data[TEST_NUM_SAMPLES];
        uint32_t i, j;

        for (i = 0; i < num_test_cases; i++) {
            const struct ButterworthFilterTestCase *ptest = &filter_test_case[i];
            int32_t work_size;
            void *work;
            struct AE2IIRFilter *filter;

            ASSERT_TRUE(ptest->frequency < (TEST_NUM_SAMPLES / 2));

            work_size = AE2IIRFilter_CalculateWorkSize(ptest->order);
            ASSERT_TRUE(work_size >= 0);
            work = malloc(work_size);
            ASSERT_TRUE(work != NULL);
            filter = AE2IIRFilter_Create(ptest->order, work, work_size);
            ASSERT_TRUE(filter != NULL);

            /* フィルタ作成 */
            AE2IIRFilter_SetButterworthLowPass(filter, ptest->order, TEST_NUM_SAMPLES, ptest->frequency);

            /* 単位インパルス生成 */
            data[0] = 1.0f;
            for (j = 1; j < TEST_NUM_SAMPLES; j++) {
                data[j] = 0.0f;
            }

            /* フィルタ適用 */
            AE2IIRFilter_ClearBuffer(filter);
            AE2IIRFilter_Process(filter, data, TEST_NUM_SAMPLES);

            /* スペクトル計算 */
            AE2IIRFilterTest_ComputeSpectrum(data, TEST_NUM_SAMPLES);

            /* 通過域で1.0, カットオフ周波数で1/sqrt(2)，遮断域で0.0に近いことを期待 */
            EXPECT_NEAR(1.0, data[0], 1e-2);
            EXPECT_NEAR(1.0 / sqrt(2.0), data[ptest->frequency], 1e-2);
            EXPECT_NEAR(0.0, data[TEST_NUM_SAMPLES / 2 - 1], 1e-2);

            /* フィルタ破棄 */
            AE2IIRFilter_Destroy(filter);
            free(work);
        }
    }

    /* ハイパスフィルタのテスト */
    {
        float data[TEST_NUM_SAMPLES];
        uint32_t i, j;

        for (i = 0; i < num_test_cases; i++) {
            const struct ButterworthFilterTestCase *ptest = &filter_test_case[i];
            int32_t work_size;
            void *work;
            struct AE2IIRFilter *filter;

            ASSERT_TRUE(ptest->frequency < (TEST_NUM_SAMPLES / 2));

            work_size = AE2IIRFilter_CalculateWorkSize(ptest->order);
            ASSERT_TRUE(work_size >= 0);
            work = malloc(work_size);
            ASSERT_TRUE(work != NULL);
            filter = AE2IIRFilter_Create(ptest->order, work, work_size);
            ASSERT_TRUE(filter != NULL);

            /* フィルタ作成 */
            AE2IIRFilter_SetButterworthHighPass(filter, ptest->order, TEST_NUM_SAMPLES, ptest->frequency);

            /* 単位インパルス生成 */
            data[0] = 1.0f;
            for (j = 1; j < TEST_NUM_SAMPLES; j++) {
                data[j] = 0.0f;
            }

            /* フィルタ適用 */
            AE2IIRFilter_ClearBuffer(filter);
            AE2IIRFilter_Process(filter, data, TEST_NUM_SAMPLES);

            /* スペクトル計算 */
            AE2IIRFilterTest_ComputeSpectrum(data, TEST_NUM_SAMPLES);

            /* 通過域で1.0, カットオフ周波数で1/sqrt(2)，遮断域で0.0に近いことを期待 */
            EXPECT_NEAR(0.0, data[0], 1e-2);
            EXPECT_NEAR(1.0 / sqrt(2.0), data[ptest->frequency], 1e-2);
            EXPECT_NEAR(1.0, data[TEST_NUM_SAMPLES / 2 - 1], 1e-2);

            /* フィルタ破棄 */
            AE2IIRFilter_Destroy(filter);
            free(work);
        }
    }

#undef TEST_NUM_SAMPLES
}

/* チェビシェフフィルタデザインテスト */
TEST(AE2IIRFilterTest, ChebyshevFilterDesignTest)
{
#define TEST_NUM_SAMPLES 1024

    /* テストケース */
    static const struct ChebyshevFilterTestCase {
        int32_t order; /* 次数 */
        int32_t frequency; /* 周波数（注目するビン） */
        float ripple; /* リップル(dB) */
    } filter_test_case[] = {
        {  2,       TEST_NUM_SAMPLES / 8, 0.1 },
        {  2,       TEST_NUM_SAMPLES / 8, 1.0 },
        {  2,       TEST_NUM_SAMPLES / 8, 6.0 },
        {  2,       TEST_NUM_SAMPLES / 8, 0.1 },
        {  2,       TEST_NUM_SAMPLES / 4, 1.0 },
        {  2,       TEST_NUM_SAMPLES / 4, 6.0 },
        {  2,       TEST_NUM_SAMPLES / 4, 0.1 },
        {  2,       TEST_NUM_SAMPLES / 4, 1.0 },
        {  2, (3 * TEST_NUM_SAMPLES) / 8, 6.0 },
        {  2, (3 * TEST_NUM_SAMPLES) / 8, 0.1 },
        {  2, (3 * TEST_NUM_SAMPLES) / 8, 1.0 },
        {  2, (3 * TEST_NUM_SAMPLES) / 8, 6.0 },
        {  3,       TEST_NUM_SAMPLES / 8, 0.1 },
        {  3,       TEST_NUM_SAMPLES / 8, 1.0 },
        {  3,       TEST_NUM_SAMPLES / 8, 6.0 },
        {  3,       TEST_NUM_SAMPLES / 8, 0.1 },
        {  3,       TEST_NUM_SAMPLES / 4, 1.0 },
        {  3,       TEST_NUM_SAMPLES / 4, 6.0 },
        {  3,       TEST_NUM_SAMPLES / 4, 0.1 },
        {  3,       TEST_NUM_SAMPLES / 4, 1.0 },
        {  3, (3 * TEST_NUM_SAMPLES) / 8, 6.0 },
        {  3, (3 * TEST_NUM_SAMPLES) / 8, 0.1 },
        {  3, (3 * TEST_NUM_SAMPLES) / 8, 1.0 },
        {  3, (3 * TEST_NUM_SAMPLES) / 8, 6.0 },
        { 20,       TEST_NUM_SAMPLES / 8, 0.1 },
        { 20,       TEST_NUM_SAMPLES / 8, 1.0 },
        { 20,       TEST_NUM_SAMPLES / 8, 6.0 },
        { 20,       TEST_NUM_SAMPLES / 8, 0.1 },
        { 20,       TEST_NUM_SAMPLES / 4, 1.0 },
        { 20,       TEST_NUM_SAMPLES / 4, 6.0 },
        { 20,       TEST_NUM_SAMPLES / 4, 0.1 },
        { 20,       TEST_NUM_SAMPLES / 4, 1.0 },
        { 20, (3 * TEST_NUM_SAMPLES) / 8, 6.0 },
        { 20, (3 * TEST_NUM_SAMPLES) / 8, 0.1 },
        { 20, (3 * TEST_NUM_SAMPLES) / 8, 1.0 },
        { 20, (3 * TEST_NUM_SAMPLES) / 8, 6.0 },
        { 21,       TEST_NUM_SAMPLES / 8, 0.1 },
        { 21,       TEST_NUM_SAMPLES / 8, 1.0 },
        { 21,       TEST_NUM_SAMPLES / 8, 6.0 },
        { 21,       TEST_NUM_SAMPLES / 8, 0.1 },
        { 21,       TEST_NUM_SAMPLES / 4, 1.0 },
        { 21,       TEST_NUM_SAMPLES / 4, 6.0 },
        { 21,       TEST_NUM_SAMPLES / 4, 0.1 },
        { 21,       TEST_NUM_SAMPLES / 4, 1.0 },
        { 21, (3 * TEST_NUM_SAMPLES) / 8, 6.0 },
        { 21, (3 * TEST_NUM_SAMPLES) / 8, 0.1 },
        { 21, (3 * TEST_NUM_SAMPLES) / 8, 1.0 },
        { 21, (3 * TEST_NUM_SAMPLES) / 8, 6.0 },
    };
    static const uint32_t num_test_cases = sizeof(filter_test_case) / sizeof(filter_test_case[0]);

    /* ローパスフィルタのテスト */
    {
        float data[TEST_NUM_SAMPLES];
        uint32_t i, j;

        for (i = 0; i < num_test_cases; i++) {
            const struct ChebyshevFilterTestCase *ptest = &filter_test_case[i];
            int32_t work_size;
            void *work;
            struct AE2IIRFilter *filter;

            ASSERT_TRUE(ptest->frequency < (TEST_NUM_SAMPLES / 2));

            work_size = AE2IIRFilter_CalculateWorkSize(ptest->order);
            ASSERT_TRUE(work_size >= 0);
            work = malloc(work_size);
            ASSERT_TRUE(work != NULL);
            filter = AE2IIRFilter_Create(ptest->order, work, work_size);
            ASSERT_TRUE(filter != NULL);

            /* フィルタ作成 */
            AE2IIRFilter_SetChebyshevLowPass(filter, ptest->order, TEST_NUM_SAMPLES, ptest->frequency, ptest->ripple);

            /* 単位インパルス生成 */
            data[0] = 1.0f;
            for (j = 1; j < TEST_NUM_SAMPLES; j++) {
                data[j] = 0.0f;
            }

            /* フィルタ適用 */
            AE2IIRFilter_ClearBuffer(filter);
            AE2IIRFilter_Process(filter, data, TEST_NUM_SAMPLES);

            /* スペクトル計算 */
            AE2IIRFilterTest_ComputeSpectrum(data, TEST_NUM_SAMPLES);

            /* 通過域で1.0, 遮断域で0.0に近いことを期待 */
            {
                int32_t is_ok = 1;
                for (j = 0; j < ptest->frequency; j++) {
                    const float gaindB = fabs(20.0 * log10(data[j]));
                    /* 数値演算誤差で設計値よりも大きくなることがあるため、マージンを入れる */
                    if (gaindB > (ptest->ripple * 1.1)) {
                        is_ok = 0;
                        break;
                    }
                }
                EXPECT_EQ(1, is_ok);
            }
            EXPECT_NEAR(0.0, data[TEST_NUM_SAMPLES / 2 - 1], 1e-2);

            /* フィルタ破棄 */
            AE2IIRFilter_Destroy(filter);
            free(work);
        }
    }

    /* ハイパスフィルタのテスト */
    {
        float data[TEST_NUM_SAMPLES];
        uint32_t i, j;

        for (i = 0; i < num_test_cases; i++) {
            const struct ChebyshevFilterTestCase *ptest = &filter_test_case[i];
            int32_t work_size;
            void *work;
            struct AE2IIRFilter *filter;

            ASSERT_TRUE(ptest->frequency < (TEST_NUM_SAMPLES / 2));

            work_size = AE2IIRFilter_CalculateWorkSize(ptest->order);
            ASSERT_TRUE(work_size >= 0);
            work = malloc(work_size);
            ASSERT_TRUE(work != NULL);
            filter = AE2IIRFilter_Create(ptest->order, work, work_size);
            ASSERT_TRUE(filter != NULL);

            /* フィルタ作成 */
            AE2IIRFilter_SetChebyshevHighPass(filter, ptest->order, TEST_NUM_SAMPLES, ptest->frequency, ptest->ripple);

            /* 単位インパルス生成 */
            data[0] = 1.0f;
            for (j = 1; j < TEST_NUM_SAMPLES; j++) {
                data[j] = 0.0f;
            }

            /* フィルタ適用 */
            AE2IIRFilter_ClearBuffer(filter);
            AE2IIRFilter_Process(filter, data, TEST_NUM_SAMPLES);

            /* スペクトル計算 */
            AE2IIRFilterTest_ComputeSpectrum(data, TEST_NUM_SAMPLES);

            /* 通過域で1.0, 遮断域で0.0に近いことを期待 */
            {
                int32_t is_ok = 1;
                for (j = ptest->frequency + 1; j < TEST_NUM_SAMPLES / 2; j++) {
                    const float gaindB = fabs(20.0 * log10(data[j]));
                    /* 数値演算誤差で設計値よりも大きくなることがあるため、マージンを入れる */
                    if (gaindB > (ptest->ripple * 1.1)) {
                        is_ok = 0;
                        break;
                    }
                }
                EXPECT_EQ(1, is_ok);
            }
            EXPECT_NEAR(0.0, data[0], 1e-2);

            /* フィルタ破棄 */
            AE2IIRFilter_Destroy(filter);
            free(work);
        }
    }

#undef TEST_NUM_SAMPLES
}

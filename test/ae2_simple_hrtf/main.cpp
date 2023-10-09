#include <stdlib.h>
#include <string.h>

#include <gtest/gtest.h>

/* テスト対象のモジュール */
extern "C" {
#include "../../libs/ae2_simple_hrtf/src/ae2_simple_hrtf.c"
}

/* ハンドル作成破棄テスト */
TEST(AE2SimpleHRTFTest, CreateDestroyTest)
{
    /* ワークサイズ計算テスト */
    {
        int32_t work_size;
        AE2SimpleHRTFConfig config;

        /* 簡単な成功例 */
        config.max_num_process_samples = 1;
        config.max_radius_of_head = AE2SIMPLEHRTF_DEFAULT_RADIUS_OF_HEAD;
        config.sampling_rate = 1.0f;
        config.delay_fade_time_ms = 0.0f;
        work_size = AE2SimpleHRTF_CalculateWorkSize(&config);
        EXPECT_TRUE(work_size >= (int32_t)sizeof(struct AE2SimpleHRTF));

        /* 不正な次数 */
        work_size = AE2SimpleHRTF_CalculateWorkSize(NULL);
        EXPECT_TRUE(work_size < 0);
    }

    /* ワーク領域渡しによるハンドル作成（成功例） */
    {
        void *work;
        int32_t work_size;
        AE2SimpleHRTFConfig config;
        struct AE2SimpleHRTF *hrtf;

        config.max_num_process_samples = 1;
        config.max_radius_of_head = AE2SIMPLEHRTF_DEFAULT_RADIUS_OF_HEAD;
        config.sampling_rate = 1.0f;
        config.delay_fade_time_ms = 0.0f;
        work_size = AE2SimpleHRTF_CalculateWorkSize(&config);
        work = malloc(work_size);

        hrtf = AE2SimpleHRTF_Create(&config, work, work_size);
        EXPECT_TRUE(hrtf != NULL);

        AE2SimpleHRTF_Destroy(hrtf);
        free(work);
    }

    /* ワーク領域渡しによるハンドル作成（失敗ケース） */
    {
        void *work;
        int32_t work_size;
        AE2SimpleHRTFConfig config;
        struct AE2SimpleHRTF *hrtf;

        config.max_num_process_samples = 1;
        config.max_radius_of_head = AE2SIMPLEHRTF_DEFAULT_RADIUS_OF_HEAD;
        config.sampling_rate = 1.0f;
        config.delay_fade_time_ms = 0.0f;
        work_size = AE2SimpleHRTF_CalculateWorkSize(&config);
        work = malloc(work_size);

        /* 引数が不正 */
        hrtf = AE2SimpleHRTF_Create(&config, NULL, work_size);
        EXPECT_TRUE(hrtf == NULL);
        hrtf = AE2SimpleHRTF_Create(&config, work, 0);
        EXPECT_TRUE(hrtf == NULL);

        /* ワークサイズ不足 */
        hrtf = AE2SimpleHRTF_Create(&config, work, work_size - 1);
        EXPECT_TRUE(hrtf == NULL);

        free(work);
    }
}

/* 角度設定テスト */
TEST(AE2SimpleHRTFTest, SetAngleTest)
{
#define NUM_TEST_SAMPLES 256
    {
        void *work;
        float angle;
        int32_t work_size, smpl;
        AE2SimpleHRTFConfig config;
        struct AE2SimpleHRTF *hrtf;
        float data[NUM_TEST_SAMPLES], data2[NUM_TEST_SAMPLES];

        config.max_num_process_samples = NUM_TEST_SAMPLES;
        config.max_radius_of_head = AE2SIMPLEHRTF_DEFAULT_RADIUS_OF_HEAD;
        config.sampling_rate = 44100.0f;
        config.delay_fade_time_ms = 0.0f;
        work_size = AE2SimpleHRTF_CalculateWorkSize(&config);
        work = malloc(work_size);

        hrtf = AE2SimpleHRTF_Create(&config, work, work_size);
        ASSERT_TRUE(hrtf != NULL);

        /* 対称性より、角度を反転した結果はあっているべき */
        for (angle = 0.0; angle <= (AE2_PI + FLT_EPSILON); angle += AE2_PI / 6.0) {
            data[0] = data2[0] = 1.0f;
            for (smpl = 1; smpl < NUM_TEST_SAMPLES; smpl++) {
                data[smpl] = data2[smpl] = 0.0f;
            }

            AE2SimpleHRTF_SetAzimuthAngle(hrtf, angle);
            AE2SimpleHRTF_Process(hrtf, data, NUM_TEST_SAMPLES);
            AE2SimpleHRTF_SetAzimuthAngle(hrtf, -angle);
            AE2SimpleHRTF_Process(hrtf, data2, NUM_TEST_SAMPLES);

            {
                int32_t is_ok = 1;
                for (smpl = 0; smpl < NUM_TEST_SAMPLES; smpl++) {
                    if (fabs(data[smpl] - data2[smpl]) > 0.001f) {
                        is_ok = 0;
                        break;
                    }
                }
                EXPECT_EQ(1, is_ok);
            }
        }

        AE2SimpleHRTF_Destroy(hrtf);
        free(work);
    }
}

int main(int argc, char **argv)
{
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}

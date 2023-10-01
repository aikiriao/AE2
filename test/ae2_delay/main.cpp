#include <stdlib.h>
#include <string.h>

#include <gtest/gtest.h>

/* テスト対象のモジュール */
extern "C" {
#include "../../libs/ae2_delay/src/ae2_delay.c"
}

/* ハンドル作成破棄テスト */
TEST(AE2DelayTest, CreateDestroyTest)
{
    /* ワークサイズ計算テスト */
    {
        int32_t work_size;
        AE2DelayConfig config;

        /* 簡単な成功例 */
        config.max_num_delay_samples = 1;
        config.max_num_process_samples = 1;
        work_size = AE2Delay_CalculateWorkSize(&config);
        EXPECT_TRUE(work_size >= (int32_t)sizeof(struct AE2Delay));

        /* 不正な次数 */
        work_size = AE2Delay_CalculateWorkSize(NULL);
        EXPECT_TRUE(work_size < 0);
    }

    /* ワーク領域渡しによるハンドル作成（成功例） */
    {
        void *work;
        int32_t work_size;
        AE2DelayConfig config;
        struct AE2Delay *delay;

        config.max_num_delay_samples = 1;
        config.max_num_process_samples = 1;
        work_size = AE2Delay_CalculateWorkSize(&config);
        work = malloc(work_size);

        delay = AE2Delay_Create(&config, work, work_size);
        EXPECT_TRUE(delay != NULL);

        AE2Delay_Destroy(delay);
        free(work);
    }

    /* ワーク領域渡しによるハンドル作成（失敗ケース） */
    {
        void *work;
        int32_t work_size;
        AE2DelayConfig config;
        struct AE2Delay *delay;

        config.max_num_delay_samples = 1;
        config.max_num_process_samples = 1;
        work_size = AE2Delay_CalculateWorkSize(&config);
        work = malloc(work_size);

        /* 引数が不正 */
        delay = AE2Delay_Create(&config, NULL, work_size);
        EXPECT_TRUE(delay == NULL);
        delay = AE2Delay_Create(&config, work, 0);
        EXPECT_TRUE(delay == NULL);

        /* ワークサイズ不足 */
        delay = AE2Delay_Create(&config, work, work_size - 1);
        EXPECT_TRUE(delay == NULL);

        free(work);
    }
}


/* 遅延テスト */
TEST(AE2DelayTest, DelayTest)
{
#define NUM_TEST_DATA_SAMPLES 50

    /* 単一ディレイテスト */
    {
        void *work;
        int32_t work_size, smpl, num_delay_samples;
        AE2DelayConfig config;
        struct AE2Delay *delay;
        float test_data[NUM_TEST_DATA_SAMPLES];

        config.max_num_delay_samples = NUM_TEST_DATA_SAMPLES;
        config.max_num_process_samples = 10;
        work_size = AE2Delay_CalculateWorkSize(&config);
        work = malloc(work_size);

        delay = AE2Delay_Create(&config, work, work_size);
        ASSERT_TRUE(delay != NULL);

        /* インデックスに応じて増えていくデータを生成 */
        for (smpl = 0; smpl < NUM_TEST_DATA_SAMPLES; smpl++) {
            test_data[smpl] = smpl + 1;
        }

        /* 遅延量を変えながらテスト */
        for (num_delay_samples = 0;
            num_delay_samples <= config.max_num_delay_samples;
            num_delay_samples++) {
            float data[NUM_TEST_DATA_SAMPLES];

            /* 遅延量設定 */
            AE2Delay_Reset(delay);
            AE2Delay_SetDelay(delay, num_delay_samples, AE2DELAY_FADETYPE_LINEAR, 0);

            /* 信号処理 */
            memcpy(data, test_data, sizeof(float) * NUM_TEST_DATA_SAMPLES);
            smpl = 0;
            while (smpl < NUM_TEST_DATA_SAMPLES) {
                const int32_t num_process_samples
                    = AE2DELAY_MIN(config.max_num_process_samples, NUM_TEST_DATA_SAMPLES - smpl);
                AE2Delay_Process(delay, &data[smpl], num_process_samples);
                smpl += num_process_samples;
            }

            /* 遅延分の0の後に、遅延した元データが現れるか確認 */
            {
                int32_t is_ok = 1;
                for (smpl = 0; smpl < NUM_TEST_DATA_SAMPLES; smpl++) {
                    if (smpl < num_delay_samples) {
                        if (data[smpl] != 0.0f) {
                            is_ok = 0;
                            break;
                        }
                    } else {
                        if (data[smpl] != test_data[smpl - num_delay_samples]) {
                            is_ok = 0;
                            break;
                        }
                    }
                }
                EXPECT_EQ(1, is_ok);
            }
        }

        AE2Delay_Destroy(delay);
        free(work);
    }

    /* ディレイ量変更テスト */
    {
        void *work;
        int32_t work_size, smpl, num_delay_samples, num_feed_samples, fade_type;
        AE2DelayConfig config;
        struct AE2Delay *delay;
        float test_data[NUM_TEST_DATA_SAMPLES];

        config.max_num_delay_samples = NUM_TEST_DATA_SAMPLES;
        config.max_num_process_samples = 10;
        work_size = AE2Delay_CalculateWorkSize(&config);
        work = malloc(work_size);

        delay = AE2Delay_Create(&config, work, work_size);
        ASSERT_TRUE(delay != NULL);

        for (smpl = 0; smpl < NUM_TEST_DATA_SAMPLES; smpl++) {
            test_data[smpl] = 1.0f;
        }

        /* 遅延量を変えながらテスト */
        for (num_delay_samples = 0;
            num_delay_samples <= config.max_num_delay_samples;
            num_delay_samples++) {
            for (num_feed_samples = 1;
                num_feed_samples <= num_delay_samples;
                num_feed_samples++) {
                for (fade_type = AE2DELAY_FADETYPE_LINEAR;
                    fade_type <= AE2DELAY_FADETYPE_SQUARE;
                    fade_type++) {
                    float data[NUM_TEST_DATA_SAMPLES];

                    /* 遅延量設定 */
                    AE2Delay_Reset(delay);
                    /* フェード設定しつつディレイ */
                    AE2Delay_SetDelay(delay,
                        num_delay_samples, (AE2DelayFadeType)fade_type, num_feed_samples);

                    /* 信号処理 */
                    memcpy(data, test_data, sizeof(float) * NUM_TEST_DATA_SAMPLES);
                    smpl = 0;
                    while (smpl < NUM_TEST_DATA_SAMPLES) {
                        const int32_t num_process_samples
                            = AE2DELAY_MIN(config.max_num_process_samples, NUM_TEST_DATA_SAMPLES - smpl);
                        AE2Delay_Process(delay, &data[smpl], num_process_samples);
                        smpl += num_process_samples;
                    }

                    /* 0.0fが続くディレイデータと1.0fが続くデータを補間するので、
                    1.0から始まってフェードが終わるまでデータは減少し続けるはず */
                    {
                        int32_t is_ok = 1;
                        for (smpl = 1; smpl < num_feed_samples; smpl++) {
                            if (smpl < num_delay_samples) {
                                if (data[smpl] > data[smpl - 1]) {
                                    is_ok = 0;
                                    break;
                                }
                            }
                        }
                        EXPECT_EQ(1, is_ok);
                    }
                }
            }
        }

        AE2Delay_Destroy(delay);
        free(work);
    }
#undef NUM_TEST_DATA_SAMPLES
}

int main(int argc, char **argv)
{
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}

#include <stdlib.h>
#include <string.h>

#include <gtest/gtest.h>

/* テスト対象のモジュール */
extern "C" {
#include "../../libs/ae2_ring_buffer/src/ae2_ring_buffer.c"
}

/* ハンドル作成破棄テスト */
TEST(AE2RighBufferTest, CreateDestroyTest)
{
    /* ワークサイズ計算テスト */
    {
        int32_t work_size;
        AE2RingBufferConfig config;

        /* 簡単な成功例 */
        config.max_required_ndata = 1;
        config.max_ndata = 1;
        config.data_unit_size = 1;
        work_size = AE2RingBuffer_CalculateWorkSize(&config);
        EXPECT_TRUE(work_size >= (int32_t)sizeof(struct AE2RingBuffer));

        /* 不正な次数 */
        work_size = AE2RingBuffer_CalculateWorkSize(NULL);
        EXPECT_TRUE(work_size < 0);
    }

    /* ワーク領域渡しによるハンドル作成（成功例） */
    {
        void *work;
        int32_t work_size;
        AE2RingBufferConfig config;
        struct AE2RingBuffer *buffer;

        config.max_required_ndata = 1;
        config.max_ndata = 1;
        config.data_unit_size = 1;
        work_size = AE2RingBuffer_CalculateWorkSize(&config);
        work = malloc(work_size);

        buffer = AE2RingBuffer_Create(&config, work, work_size);
        EXPECT_TRUE(buffer != NULL);

        AE2RingBuffer_Destroy(buffer);
        free(work);
    }

    /* ワーク領域渡しによるハンドル作成（失敗ケース） */
    {
        void *work;
        int32_t work_size;
        AE2RingBufferConfig config;
        struct AE2RingBuffer *buffer;

        config.max_required_ndata = 1;
        config.max_ndata = 1;
        config.data_unit_size = 1;
        work_size = AE2RingBuffer_CalculateWorkSize(&config);
        work = malloc(work_size);

        /* 引数が不正 */
        buffer = AE2RingBuffer_Create(&config, NULL, work_size);
        EXPECT_TRUE(buffer == NULL);
        buffer = AE2RingBuffer_Create(&config, work, 0);
        EXPECT_TRUE(buffer == NULL);

        /* ワークサイズ不足 */
        buffer = AE2RingBuffer_Create(&config, work, work_size - 1);
        EXPECT_TRUE(buffer == NULL);

        free(work);
    }
}

/* Put / Getテスト */
TEST(AE2RingBufferTest, PutGetTest)
{
    {
        int32_t work_size;
        void *work;
        struct AE2RingBuffer *buf;
        struct AE2RingBufferConfig config;
        const char data[] = "0123456789";
        char *tmp;

        config.max_ndata = 6;
        config.max_required_ndata = 3;
        config.data_unit_size = sizeof(uint8_t);

        work_size = AE2RingBuffer_CalculateWorkSize(&config);
        ASSERT_TRUE(work_size >= 0);
        work = malloc(work_size);

        buf = AE2RingBuffer_Create(&config, work, work_size);
        ASSERT_TRUE(buf != NULL);

        EXPECT_EQ(0, AE2RingBuffer_GetRemainNumData(buf));
        EXPECT_EQ(6, AE2RingBuffer_GetCapacityNumData(buf));

        EXPECT_EQ(AE2RINGBUFFER_APIRESULT_OK, AE2RingBuffer_Put(buf, data, 1));
        EXPECT_EQ(1, AE2RingBuffer_GetRemainNumData(buf));
        EXPECT_EQ(5, AE2RingBuffer_GetCapacityNumData(buf));
        EXPECT_EQ(AE2RINGBUFFER_APIRESULT_OK, AE2RingBuffer_Peek(buf, (void **)&tmp, 1));
        EXPECT_EQ(tmp[0], data[0]);
        EXPECT_EQ(1, AE2RingBuffer_GetRemainNumData(buf));
        EXPECT_EQ(5, AE2RingBuffer_GetCapacityNumData(buf));
        EXPECT_EQ(AE2RINGBUFFER_APIRESULT_OK, AE2RingBuffer_Get(buf, (void **)&tmp, 1));
        EXPECT_EQ(tmp[0], data[0]);
        EXPECT_EQ(0, AE2RingBuffer_GetRemainNumData(buf));
        EXPECT_EQ(6, AE2RingBuffer_GetCapacityNumData(buf));

        EXPECT_EQ(AE2RINGBUFFER_APIRESULT_OK, AE2RingBuffer_Put(buf, data, 6));
        EXPECT_EQ(6, AE2RingBuffer_GetRemainNumData(buf));
        EXPECT_EQ(0, AE2RingBuffer_GetCapacityNumData(buf));
        EXPECT_EQ(AE2RINGBUFFER_APIRESULT_OK, AE2RingBuffer_Peek(buf, (void **)&tmp, 3));
        EXPECT_EQ(0, memcmp(tmp, &data[0], 3));
        EXPECT_EQ(6, AE2RingBuffer_GetRemainNumData(buf));
        EXPECT_EQ(0, AE2RingBuffer_GetCapacityNumData(buf));
        EXPECT_EQ(AE2RINGBUFFER_APIRESULT_OK, AE2RingBuffer_Get(buf, (void **)&tmp, 3));
        EXPECT_EQ(0, memcmp(tmp, &data[0], 3));
        EXPECT_EQ(3, AE2RingBuffer_GetRemainNumData(buf));
        EXPECT_EQ(3, AE2RingBuffer_GetCapacityNumData(buf));
        EXPECT_EQ(AE2RINGBUFFER_APIRESULT_OK, AE2RingBuffer_DelayedPeek(buf, (void **)&tmp, 3, 3));
        EXPECT_EQ(0, memcmp(tmp, &data[0], 3));
        EXPECT_EQ(3, AE2RingBuffer_GetRemainNumData(buf));
        EXPECT_EQ(3, AE2RingBuffer_GetCapacityNumData(buf));
        EXPECT_EQ(AE2RINGBUFFER_APIRESULT_OK, AE2RingBuffer_Get(buf, (void **)&tmp, 3));
        EXPECT_EQ(0, memcmp(tmp, &data[3], 3));
        EXPECT_EQ(0, AE2RingBuffer_GetRemainNumData(buf));
        EXPECT_EQ(6, AE2RingBuffer_GetCapacityNumData(buf));
        EXPECT_EQ(AE2RINGBUFFER_APIRESULT_OK, AE2RingBuffer_DelayedPeek(buf, (void **)&tmp, 3, 6));
        EXPECT_EQ(0, memcmp(tmp, &data[0], 3));
        EXPECT_EQ(0, AE2RingBuffer_GetRemainNumData(buf));
        EXPECT_EQ(6, AE2RingBuffer_GetCapacityNumData(buf));

        EXPECT_EQ(AE2RINGBUFFER_APIRESULT_OK, AE2RingBuffer_Put(buf, &data[0], 2));
        EXPECT_EQ(2, AE2RingBuffer_GetRemainNumData(buf));
        EXPECT_EQ(4, AE2RingBuffer_GetCapacityNumData(buf));
        EXPECT_EQ(AE2RINGBUFFER_APIRESULT_OK, AE2RingBuffer_Put(buf, &data[2], 2));
        EXPECT_EQ(4, AE2RingBuffer_GetRemainNumData(buf));
        EXPECT_EQ(2, AE2RingBuffer_GetCapacityNumData(buf));
        EXPECT_EQ(AE2RINGBUFFER_APIRESULT_OK, AE2RingBuffer_Get(buf, (void **)&tmp, 3));
        EXPECT_EQ(0, memcmp(tmp, &data[0], 3));
        EXPECT_EQ(1, AE2RingBuffer_GetRemainNumData(buf));
        EXPECT_EQ(5, AE2RingBuffer_GetCapacityNumData(buf));
        EXPECT_EQ(AE2RINGBUFFER_APIRESULT_OK, AE2RingBuffer_DelayedPeek(buf, (void **)&tmp, 3, 3));
        EXPECT_EQ(0, memcmp(tmp, &data[0], 3));
        EXPECT_EQ(1, AE2RingBuffer_GetRemainNumData(buf));
        EXPECT_EQ(5, AE2RingBuffer_GetCapacityNumData(buf));
        EXPECT_EQ(AE2RINGBUFFER_APIRESULT_OK, AE2RingBuffer_Put(buf, &data[4], 2));
        EXPECT_EQ(3, AE2RingBuffer_GetRemainNumData(buf));
        EXPECT_EQ(3, AE2RingBuffer_GetCapacityNumData(buf));
        EXPECT_EQ(AE2RINGBUFFER_APIRESULT_OK, AE2RingBuffer_Get(buf, (void **)&tmp, 3));
        EXPECT_EQ(0, memcmp(tmp, &data[3], 3));
        EXPECT_EQ(0, AE2RingBuffer_GetRemainNumData(buf));
        EXPECT_EQ(6, AE2RingBuffer_GetCapacityNumData(buf));
        EXPECT_EQ(AE2RINGBUFFER_APIRESULT_OK, AE2RingBuffer_DelayedPeek(buf, (void **)&tmp, 3, 3));
        EXPECT_EQ(0, memcmp(tmp, &data[3], 3));
        EXPECT_EQ(0, AE2RingBuffer_GetRemainNumData(buf));
        EXPECT_EQ(6, AE2RingBuffer_GetCapacityNumData(buf));

        EXPECT_EQ(AE2RINGBUFFER_APIRESULT_OK, AE2RingBuffer_Put(buf, &data[0], 5));
        EXPECT_EQ(5, AE2RingBuffer_GetRemainNumData(buf));
        EXPECT_EQ(1, AE2RingBuffer_GetCapacityNumData(buf));
        EXPECT_EQ(AE2RINGBUFFER_APIRESULT_OK, AE2RingBuffer_Get(buf, (void **)&tmp, 3));
        EXPECT_EQ(0, memcmp(tmp, &data[0], 3));
        EXPECT_EQ(2, AE2RingBuffer_GetRemainNumData(buf));
        EXPECT_EQ(4, AE2RingBuffer_GetCapacityNumData(buf));
        EXPECT_EQ(AE2RINGBUFFER_APIRESULT_OK, AE2RingBuffer_Get(buf, (void **)&tmp, 2));
        EXPECT_EQ(0, memcmp(tmp, &data[3], 2));
        EXPECT_EQ(0, AE2RingBuffer_GetRemainNumData(buf));
        EXPECT_EQ(6, AE2RingBuffer_GetCapacityNumData(buf));
        EXPECT_EQ(AE2RINGBUFFER_APIRESULT_OK, AE2RingBuffer_DelayedPeek(buf, (void **)&tmp, 3, 3));
        EXPECT_EQ(0, memcmp(tmp, &data[2], 3));
        EXPECT_EQ(0, AE2RingBuffer_GetRemainNumData(buf));
        EXPECT_EQ(6, AE2RingBuffer_GetCapacityNumData(buf));
        EXPECT_EQ(AE2RINGBUFFER_APIRESULT_OK, AE2RingBuffer_Put(buf, &data[0], 5));
        EXPECT_EQ(5, AE2RingBuffer_GetRemainNumData(buf));
        EXPECT_EQ(1, AE2RingBuffer_GetCapacityNumData(buf));
        EXPECT_EQ(AE2RINGBUFFER_APIRESULT_OK, AE2RingBuffer_Get(buf, (void **)&tmp, 3));
        EXPECT_EQ(0, memcmp(tmp, &data[0], 3));
        EXPECT_EQ(2, AE2RingBuffer_GetRemainNumData(buf));
        EXPECT_EQ(4, AE2RingBuffer_GetCapacityNumData(buf));
        EXPECT_EQ(AE2RINGBUFFER_APIRESULT_OK, AE2RingBuffer_DelayedPeek(buf, (void **)&tmp, 3, 3));
        EXPECT_EQ(0, memcmp(tmp, &data[0], 3));
        EXPECT_EQ(2, AE2RingBuffer_GetRemainNumData(buf));
        EXPECT_EQ(4, AE2RingBuffer_GetCapacityNumData(buf));
        EXPECT_EQ(AE2RINGBUFFER_APIRESULT_OK, AE2RingBuffer_Get(buf, (void **)&tmp, 2));
        EXPECT_EQ(0, memcmp(tmp, &data[3], 2));
        EXPECT_EQ(0, AE2RingBuffer_GetRemainNumData(buf));
        EXPECT_EQ(6, AE2RingBuffer_GetCapacityNumData(buf));
        EXPECT_EQ(AE2RINGBUFFER_APIRESULT_OK, AE2RingBuffer_DelayedPeek(buf, (void **)&tmp, 2, 2));
        EXPECT_EQ(0, memcmp(tmp, &data[3], 2));
        EXPECT_EQ(0, AE2RingBuffer_GetRemainNumData(buf));
        EXPECT_EQ(6, AE2RingBuffer_GetCapacityNumData(buf));

        AE2RingBuffer_Destroy(buf);
        free(buf);
    }

    /* 16bit整数で同様のテスト */
    {
        int32_t work_size;
        void *work;
        struct AE2RingBuffer *buf;
        struct AE2RingBufferConfig config;
        const int16_t data[] = { -5, -4, -3, -2, -1, 0, 1, 2, 3, 4 };
        int16_t *tmp;

        config.max_ndata = 6;
        config.max_required_ndata = 3;
        config.data_unit_size = sizeof(int16_t);

        work_size = AE2RingBuffer_CalculateWorkSize(&config);
        ASSERT_TRUE(work_size >= 0);
        work = malloc(work_size);

        buf = AE2RingBuffer_Create(&config, work, work_size);
        ASSERT_TRUE(buf != NULL);

        EXPECT_EQ(0, AE2RingBuffer_GetRemainNumData(buf));
        EXPECT_EQ(6, AE2RingBuffer_GetCapacityNumData(buf));

        EXPECT_EQ(AE2RINGBUFFER_APIRESULT_OK, AE2RingBuffer_Put(buf, data, 1));
        EXPECT_EQ(1, AE2RingBuffer_GetRemainNumData(buf));
        EXPECT_EQ(5, AE2RingBuffer_GetCapacityNumData(buf));
        EXPECT_EQ(AE2RINGBUFFER_APIRESULT_OK, AE2RingBuffer_Peek(buf, (void **)&tmp, 1));
        EXPECT_EQ(tmp[0], data[0]);
        EXPECT_EQ(1, AE2RingBuffer_GetRemainNumData(buf));
        EXPECT_EQ(5, AE2RingBuffer_GetCapacityNumData(buf));
        EXPECT_EQ(AE2RINGBUFFER_APIRESULT_OK, AE2RingBuffer_Get(buf, (void **)&tmp, 1));
        EXPECT_EQ(tmp[0], data[0]);
        EXPECT_EQ(0, AE2RingBuffer_GetRemainNumData(buf));
        EXPECT_EQ(6, AE2RingBuffer_GetCapacityNumData(buf));

        EXPECT_EQ(AE2RINGBUFFER_APIRESULT_OK, AE2RingBuffer_Put(buf, data, 6));
        EXPECT_EQ(6, AE2RingBuffer_GetRemainNumData(buf));
        EXPECT_EQ(0, AE2RingBuffer_GetCapacityNumData(buf));
        EXPECT_EQ(AE2RINGBUFFER_APIRESULT_OK, AE2RingBuffer_Peek(buf, (void **)&tmp, 3));
        EXPECT_EQ(0, memcmp(tmp, &data[0], 3 * sizeof(int16_t)));
        EXPECT_EQ(6, AE2RingBuffer_GetRemainNumData(buf));
        EXPECT_EQ(0, AE2RingBuffer_GetCapacityNumData(buf));
        EXPECT_EQ(AE2RINGBUFFER_APIRESULT_OK, AE2RingBuffer_Get(buf, (void **)&tmp, 3));
        EXPECT_EQ(0, memcmp(tmp, &data[0], 3 * sizeof(int16_t)));
        EXPECT_EQ(3, AE2RingBuffer_GetRemainNumData(buf));
        EXPECT_EQ(3, AE2RingBuffer_GetCapacityNumData(buf));
        EXPECT_EQ(AE2RINGBUFFER_APIRESULT_OK, AE2RingBuffer_DelayedPeek(buf, (void **)&tmp, 3, 3));
        EXPECT_EQ(0, memcmp(tmp, &data[0], 3 * sizeof(int16_t)));
        EXPECT_EQ(3, AE2RingBuffer_GetRemainNumData(buf));
        EXPECT_EQ(3, AE2RingBuffer_GetCapacityNumData(buf));
        EXPECT_EQ(AE2RINGBUFFER_APIRESULT_OK, AE2RingBuffer_Get(buf, (void **)&tmp, 3));
        EXPECT_EQ(0, memcmp(tmp, &data[3], 3 * sizeof(int16_t)));
        EXPECT_EQ(0, AE2RingBuffer_GetRemainNumData(buf));
        EXPECT_EQ(6, AE2RingBuffer_GetCapacityNumData(buf));
        EXPECT_EQ(AE2RINGBUFFER_APIRESULT_OK, AE2RingBuffer_DelayedPeek(buf, (void **)&tmp, 3, 6));
        EXPECT_EQ(0, memcmp(tmp, &data[0], 3 * sizeof(int16_t)));
        EXPECT_EQ(0, AE2RingBuffer_GetRemainNumData(buf));
        EXPECT_EQ(6, AE2RingBuffer_GetCapacityNumData(buf));

        EXPECT_EQ(AE2RINGBUFFER_APIRESULT_OK, AE2RingBuffer_Put(buf, &data[0], 2));
        EXPECT_EQ(2, AE2RingBuffer_GetRemainNumData(buf));
        EXPECT_EQ(4, AE2RingBuffer_GetCapacityNumData(buf));
        EXPECT_EQ(AE2RINGBUFFER_APIRESULT_OK, AE2RingBuffer_Put(buf, &data[2], 2));
        EXPECT_EQ(4, AE2RingBuffer_GetRemainNumData(buf));
        EXPECT_EQ(2, AE2RingBuffer_GetCapacityNumData(buf));
        EXPECT_EQ(AE2RINGBUFFER_APIRESULT_OK, AE2RingBuffer_Get(buf, (void **)&tmp, 3));
        EXPECT_EQ(0, memcmp(tmp, &data[0], 3 * sizeof(int16_t)));
        EXPECT_EQ(1, AE2RingBuffer_GetRemainNumData(buf));
        EXPECT_EQ(5, AE2RingBuffer_GetCapacityNumData(buf));
        EXPECT_EQ(AE2RINGBUFFER_APIRESULT_OK, AE2RingBuffer_DelayedPeek(buf, (void **)&tmp, 3, 3));
        EXPECT_EQ(0, memcmp(tmp, &data[0], 3 * sizeof(int16_t)));
        EXPECT_EQ(1, AE2RingBuffer_GetRemainNumData(buf));
        EXPECT_EQ(5, AE2RingBuffer_GetCapacityNumData(buf));
        EXPECT_EQ(AE2RINGBUFFER_APIRESULT_OK, AE2RingBuffer_Put(buf, &data[4], 2));
        EXPECT_EQ(3, AE2RingBuffer_GetRemainNumData(buf));
        EXPECT_EQ(3, AE2RingBuffer_GetCapacityNumData(buf));
        EXPECT_EQ(AE2RINGBUFFER_APIRESULT_OK, AE2RingBuffer_Get(buf, (void **)&tmp, 3));
        EXPECT_EQ(0, memcmp(tmp, &data[3], 3 * sizeof(int16_t)));
        EXPECT_EQ(0, AE2RingBuffer_GetRemainNumData(buf));
        EXPECT_EQ(6, AE2RingBuffer_GetCapacityNumData(buf));
        EXPECT_EQ(AE2RINGBUFFER_APIRESULT_OK, AE2RingBuffer_DelayedPeek(buf, (void **)&tmp, 3, 3));
        EXPECT_EQ(0, memcmp(tmp, &data[3], 3 * sizeof(int16_t)));
        EXPECT_EQ(0, AE2RingBuffer_GetRemainNumData(buf));
        EXPECT_EQ(6, AE2RingBuffer_GetCapacityNumData(buf));

        EXPECT_EQ(AE2RINGBUFFER_APIRESULT_OK, AE2RingBuffer_Put(buf, &data[0], 5));
        EXPECT_EQ(5, AE2RingBuffer_GetRemainNumData(buf));
        EXPECT_EQ(1, AE2RingBuffer_GetCapacityNumData(buf));
        EXPECT_EQ(AE2RINGBUFFER_APIRESULT_OK, AE2RingBuffer_Get(buf, (void **)&tmp, 3));
        EXPECT_EQ(0, memcmp(tmp, &data[0], 3 * sizeof(int16_t)));
        EXPECT_EQ(2, AE2RingBuffer_GetRemainNumData(buf));
        EXPECT_EQ(4, AE2RingBuffer_GetCapacityNumData(buf));
        EXPECT_EQ(AE2RINGBUFFER_APIRESULT_OK, AE2RingBuffer_Get(buf, (void **)&tmp, 2));
        EXPECT_EQ(0, memcmp(tmp, &data[3], 2 * sizeof(int16_t)));
        EXPECT_EQ(0, AE2RingBuffer_GetRemainNumData(buf));
        EXPECT_EQ(6, AE2RingBuffer_GetCapacityNumData(buf));
        EXPECT_EQ(AE2RINGBUFFER_APIRESULT_OK, AE2RingBuffer_DelayedPeek(buf, (void **)&tmp, 3, 3));
        EXPECT_EQ(0, memcmp(tmp, &data[2], 3 * sizeof(int16_t)));
        EXPECT_EQ(0, AE2RingBuffer_GetRemainNumData(buf));
        EXPECT_EQ(6, AE2RingBuffer_GetCapacityNumData(buf));
        EXPECT_EQ(AE2RINGBUFFER_APIRESULT_OK, AE2RingBuffer_Put(buf, &data[0], 5));
        EXPECT_EQ(5, AE2RingBuffer_GetRemainNumData(buf));
        EXPECT_EQ(1, AE2RingBuffer_GetCapacityNumData(buf));
        EXPECT_EQ(AE2RINGBUFFER_APIRESULT_OK, AE2RingBuffer_Get(buf, (void **)&tmp, 3));
        EXPECT_EQ(0, memcmp(tmp, &data[0], 3 * sizeof(int16_t)));
        EXPECT_EQ(2, AE2RingBuffer_GetRemainNumData(buf));
        EXPECT_EQ(4, AE2RingBuffer_GetCapacityNumData(buf));
        EXPECT_EQ(AE2RINGBUFFER_APIRESULT_OK, AE2RingBuffer_DelayedPeek(buf, (void **)&tmp, 3, 3));
        EXPECT_EQ(0, memcmp(tmp, &data[0], 3 * sizeof(int16_t)));
        EXPECT_EQ(2, AE2RingBuffer_GetRemainNumData(buf));
        EXPECT_EQ(4, AE2RingBuffer_GetCapacityNumData(buf));
        EXPECT_EQ(AE2RINGBUFFER_APIRESULT_OK, AE2RingBuffer_Get(buf, (void **)&tmp, 2));
        EXPECT_EQ(0, memcmp(tmp, &data[3], 2 * sizeof(int16_t)));
        EXPECT_EQ(0, AE2RingBuffer_GetRemainNumData(buf));
        EXPECT_EQ(6, AE2RingBuffer_GetCapacityNumData(buf));
        EXPECT_EQ(AE2RINGBUFFER_APIRESULT_OK, AE2RingBuffer_DelayedPeek(buf, (void **)&tmp, 2, 2));
        EXPECT_EQ(0, memcmp(tmp, &data[3], 2 * sizeof(int16_t)));
        EXPECT_EQ(0, AE2RingBuffer_GetRemainNumData(buf));
        EXPECT_EQ(6, AE2RingBuffer_GetCapacityNumData(buf));

        AE2RingBuffer_Destroy(buf);
        free(buf);
    }
}

int main(int argc, char **argv)
{
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}

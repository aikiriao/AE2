#include <stdlib.h>
#include <string.h>

#include <gtest/gtest.h>

/* テスト対象のモジュール */
extern "C" {
#include "../../libs/ae2_ring_buffer/src/ae2_ring_buffer.c"
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

        config.max_size = 6;
        config.max_required_size = 3;

        work_size = AE2RingBuffer_CalculateWorkSize(&config);
        ASSERT_TRUE(work_size >= 0);
        work = malloc(work_size);

        buf = AE2RingBuffer_Create(&config, work, work_size);
        ASSERT_TRUE(buf != NULL);

        EXPECT_EQ(0, AE2RingBuffer_GetRemainSize(buf));
        EXPECT_EQ(6, AE2RingBuffer_GetCapacitySize(buf));

        EXPECT_EQ(AE2RINGBUFFER_APIRESULT_OK, AE2RingBuffer_Put(buf, data, 1));
        EXPECT_EQ(1, AE2RingBuffer_GetRemainSize(buf));
        EXPECT_EQ(5, AE2RingBuffer_GetCapacitySize(buf));
        EXPECT_EQ(AE2RINGBUFFER_APIRESULT_OK, AE2RingBuffer_Get(buf, (void **)&tmp, 1));
        EXPECT_EQ(tmp[0], data[0]);
        EXPECT_EQ(0, AE2RingBuffer_GetRemainSize(buf));
        EXPECT_EQ(6, AE2RingBuffer_GetCapacitySize(buf));

        EXPECT_EQ(AE2RINGBUFFER_APIRESULT_OK, AE2RingBuffer_Put(buf, data, 6));
        EXPECT_EQ(6, AE2RingBuffer_GetRemainSize(buf));
        EXPECT_EQ(0, AE2RingBuffer_GetCapacitySize(buf));
        EXPECT_EQ(AE2RINGBUFFER_APIRESULT_OK, AE2RingBuffer_Get(buf, (void **)&tmp, 3));
        EXPECT_EQ(0, memcmp(tmp, &data[0], 3));
        EXPECT_EQ(3, AE2RingBuffer_GetRemainSize(buf));
        EXPECT_EQ(3, AE2RingBuffer_GetCapacitySize(buf));
        EXPECT_EQ(AE2RINGBUFFER_APIRESULT_OK, AE2RingBuffer_Get(buf, (void **)&tmp, 3));
        EXPECT_EQ(0, memcmp(tmp, &data[3], 3));
        EXPECT_EQ(0, AE2RingBuffer_GetRemainSize(buf));
        EXPECT_EQ(6, AE2RingBuffer_GetCapacitySize(buf));

        EXPECT_EQ(AE2RINGBUFFER_APIRESULT_OK, AE2RingBuffer_Put(buf, &data[0], 2));
        EXPECT_EQ(2, AE2RingBuffer_GetRemainSize(buf));
        EXPECT_EQ(4, AE2RingBuffer_GetCapacitySize(buf));
        EXPECT_EQ(AE2RINGBUFFER_APIRESULT_OK, AE2RingBuffer_Put(buf, &data[2], 2));
        EXPECT_EQ(4, AE2RingBuffer_GetRemainSize(buf));
        EXPECT_EQ(2, AE2RingBuffer_GetCapacitySize(buf));
        EXPECT_EQ(AE2RINGBUFFER_APIRESULT_OK, AE2RingBuffer_Get(buf, (void **)&tmp, 3));
        EXPECT_EQ(0, memcmp(tmp, &data[0], 3));
        EXPECT_EQ(1, AE2RingBuffer_GetRemainSize(buf));
        EXPECT_EQ(5, AE2RingBuffer_GetCapacitySize(buf));
        EXPECT_EQ(AE2RINGBUFFER_APIRESULT_OK, AE2RingBuffer_Put(buf, &data[4], 2));
        EXPECT_EQ(3, AE2RingBuffer_GetRemainSize(buf));
        EXPECT_EQ(3, AE2RingBuffer_GetCapacitySize(buf));
        EXPECT_EQ(AE2RINGBUFFER_APIRESULT_OK, AE2RingBuffer_Get(buf, (void **)&tmp, 3));
        EXPECT_EQ(0, memcmp(tmp, &data[3], 3));
        EXPECT_EQ(0, AE2RingBuffer_GetRemainSize(buf));
        EXPECT_EQ(6, AE2RingBuffer_GetCapacitySize(buf));

        EXPECT_EQ(AE2RINGBUFFER_APIRESULT_OK, AE2RingBuffer_Put(buf, &data[0], 5));
        EXPECT_EQ(5, AE2RingBuffer_GetRemainSize(buf));
        EXPECT_EQ(1, AE2RingBuffer_GetCapacitySize(buf));
        EXPECT_EQ(AE2RINGBUFFER_APIRESULT_OK, AE2RingBuffer_Get(buf, (void **)&tmp, 3));
        EXPECT_EQ(0, memcmp(tmp, &data[0], 3));
        EXPECT_EQ(2, AE2RingBuffer_GetRemainSize(buf));
        EXPECT_EQ(4, AE2RingBuffer_GetCapacitySize(buf));
        EXPECT_EQ(AE2RINGBUFFER_APIRESULT_OK, AE2RingBuffer_Get(buf, (void **)&tmp, 2));
        EXPECT_EQ(0, memcmp(tmp, &data[3], 2));
        EXPECT_EQ(0, AE2RingBuffer_GetRemainSize(buf));
        EXPECT_EQ(6, AE2RingBuffer_GetCapacitySize(buf));
        EXPECT_EQ(AE2RINGBUFFER_APIRESULT_OK, AE2RingBuffer_Put(buf, &data[0], 5));
        EXPECT_EQ(5, AE2RingBuffer_GetRemainSize(buf));
        EXPECT_EQ(1, AE2RingBuffer_GetCapacitySize(buf));
        EXPECT_EQ(AE2RINGBUFFER_APIRESULT_OK, AE2RingBuffer_Get(buf, (void **)&tmp, 3));
        EXPECT_EQ(0, memcmp(tmp, &data[0], 3));
        EXPECT_EQ(2, AE2RingBuffer_GetRemainSize(buf));
        EXPECT_EQ(4, AE2RingBuffer_GetCapacitySize(buf));
        EXPECT_EQ(AE2RINGBUFFER_APIRESULT_OK, AE2RingBuffer_Get(buf, (void **)&tmp, 2));
        EXPECT_EQ(0, memcmp(tmp, &data[3], 2));
        EXPECT_EQ(0, AE2RingBuffer_GetRemainSize(buf));
        EXPECT_EQ(6, AE2RingBuffer_GetCapacitySize(buf));

        AE2RingBuffer_Destroy(buf);
        free(buf);
    }
}

int main(int argc, char **argv)
{
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}

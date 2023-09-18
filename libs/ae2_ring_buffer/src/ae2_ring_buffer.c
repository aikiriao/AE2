#include "ae2_ring_buffer.h"

#include <stdio.h>
#include <string.h>
#include <assert.h>

/* メモリアラインメント */
#define AE2RINGBUFFER_ALIGNMENT 16
/* nの倍数への切り上げ */
#define AE2RINGBUFFER_ROUNDUP(val, n) ((((val) + ((n) - 1)) / (n)) * (n))
/* 最小値の取得 */
#define AE2RINGBUFFER_MIN(a,b) (((a) < (b)) ? (a) : (b))

/* リングバッファ */
struct AE2RingBuffer {
    uint8_t *data; /* データ領域の先頭ポインタ データは8ビットデータ列と考える */
    size_t buffer_size; /* バッファデータサイズ */
    size_t max_required_size; /* 最大要求データサイズ */
    uint32_t read_pos; /* 読み出し位置 */
    uint32_t write_pos; /* 書き出し位置 */
};

/* リングバッファ作成に必要なワークサイズ計算 */
int32_t AE2RingBuffer_CalculateWorkSize(const struct AE2RingBufferConfig *config)
{
    int32_t work_size;

    /* 引数チェック */
    if (config == NULL) {
        return -1;
    }
    
    /* バッファサイズは要求サイズより大きい */
    if (config->max_size < config->max_required_size) {
        return -1;
    }

    work_size = sizeof(struct AE2RingBuffer) + AE2RINGBUFFER_ALIGNMENT;
    work_size += config->max_size + 1 + AE2RINGBUFFER_ALIGNMENT;
    work_size += config->max_required_size;

    return work_size;
}

/* リングバッファ作成 */
struct AE2RingBuffer *AE2RingBuffer_Create(const struct AE2RingBufferConfig *config, void *work, int32_t work_size)
{
    struct AE2RingBuffer *buffer;
    uint8_t *work_ptr;

    /* 引数チェック */
    if ((config == NULL) || (work == NULL) || (work_size < 0)) {
        return NULL;
    }

    if (work_size < AE2RingBuffer_CalculateWorkSize(config)) {
        return NULL;
    }

    /* ハンドル領域割当 */
    work_ptr = (uint8_t *)AE2RINGBUFFER_ROUNDUP((uintptr_t)work, AE2RINGBUFFER_ALIGNMENT);
    buffer = (struct AE2RingBuffer *)work_ptr;
    work_ptr += sizeof(struct AE2RingBuffer);

    /* サイズを記録 */
    buffer->buffer_size = config->max_size + 1; /* バッファの位置関係を正しく解釈するため1要素分多く確保する（write_pos == read_pos のときデータが一杯なのか空なのか判定できない） */
    buffer->max_required_size = config->max_required_size;

    /* バッファ領域割当 */
    work_ptr = (uint8_t *)AE2RINGBUFFER_ROUNDUP((uintptr_t)work_ptr, AE2RINGBUFFER_ALIGNMENT);
    buffer->data = work_ptr;
    work_ptr += (buffer->buffer_size + config->max_required_size);

    /* バッファの内容をクリア */
    AE2RingBuffer_Clear(buffer);

    return buffer;
}

/* リングバッファ破棄 */
void AE2RingBuffer_Destroy(struct AE2RingBuffer *buffer)
{
    assert(buffer != NULL);

    /* 不定領域アクセス防止のため内容はクリア */
    AE2RingBuffer_Clear(buffer);
}

/* リングバッファの内容をクリア */
void AE2RingBuffer_Clear(struct AE2RingBuffer *buffer)
{
    assert(buffer != NULL);

    /* データ領域を0埋め */
    memset(buffer->data, 0, buffer->buffer_size + buffer->max_required_size);

    /* バッファ参照位置を初期化 */
    buffer->read_pos = 0;
    buffer->write_pos = 0;
}

/* リングバッファ内に残ったデータサイズ取得 */
size_t AE2RingBuffer_GetRemainSize(const struct AE2RingBuffer *buffer)
{
    assert(buffer != NULL);

    if (buffer->read_pos > buffer->write_pos) {
        return buffer->buffer_size + buffer->write_pos - buffer->read_pos;
    }

    return buffer->write_pos - buffer->read_pos;
}

/* リングバッファ内の空き領域サイズ取得 */
size_t AE2RingBuffer_GetCapacitySize(const struct AE2RingBuffer *buffer)
{
    assert(buffer != NULL);
    assert(buffer->buffer_size >= AE2RingBuffer_GetRemainSize(buffer));

    /* 実際に入るサイズはバッファサイズより1バイト少ない */
    return buffer->buffer_size - AE2RingBuffer_GetRemainSize(buffer) - 1;
}

/* データ挿入 */
AE2RingBufferApiResult AE2RingBuffer_Put(
        struct AE2RingBuffer *buffer, const void *data, size_t size)
{
    /* 引数チェック */
    if ((buffer == NULL) || (data == NULL) || (size == 0)) {
        return AE2RINGBUFFER_APIRESULT_INVALID_ARGUMENT;
    }

    /* バッファに空き領域がない */
    if (size > AE2RingBuffer_GetCapacitySize(buffer)) {
        return AE2RINGBUFFER_APIRESULT_EXCEED_MAX_CAPACITY;
    }

    /* リングバッファを巡回するケース: バッファ末尾までまず書き込み */
    if (buffer->write_pos + size >= buffer->buffer_size) {
        uint8_t *wp = buffer->data + buffer->write_pos;
        const size_t data_head_size = buffer->buffer_size - buffer->write_pos;
        memcpy(wp, data, data_head_size);
        data = (const void *)((uint8_t *)data + data_head_size);
        size -= data_head_size;
        buffer->write_pos = 0;
    }

    /* 剰余領域への書き込み */
    if (buffer->write_pos < buffer->max_required_size) {
        uint8_t *wp = buffer->data + buffer->buffer_size + buffer->write_pos;
        const size_t copy_size = AE2RINGBUFFER_MIN(size, buffer->max_required_size - buffer->write_pos);
        memcpy(wp, data, copy_size);
    }

    /* リングバッファへの書き込み */
    memcpy(buffer->data + buffer->write_pos, data, size);
    buffer->write_pos += size; /* 巡回するケースでインデックスの剰余処理済 */

    return AE2RINGBUFFER_APIRESULT_OK;
}

/* データ見るだけ（バッファの状態は更新されない） 注意）バッファが一周する前に使用しないと上書きされる */
AE2RingBufferApiResult AE2RingBuffer_Peek(
        const struct AE2RingBuffer *buffer, void **pdata, size_t required_size)
{
    /* 引数チェック */
    if ((buffer == NULL) || (pdata == NULL) || (required_size == 0)) {
        return AE2RINGBUFFER_APIRESULT_INVALID_ARGUMENT;
    }

    /* 最大要求サイズを超えている */
    if (required_size > buffer->max_required_size) {
        return AE2RINGBUFFER_APIRESULT_EXCEED_MAX_REQUIRED;
    }

    /* 残りデータサイズを超えている */
    if (required_size > AE2RingBuffer_GetRemainSize(buffer)) {
        return AE2RINGBUFFER_APIRESULT_EXCEED_MAX_REMAIN;
    }

    /* データの参照取得 */
    (*pdata) = (void *)(buffer->data + buffer->read_pos);

    return AE2RINGBUFFER_APIRESULT_OK;
}

/* データ取得 注意）バッファが一周する前に使用しないと上書きされる */
AE2RingBufferApiResult AE2RingBuffer_Get(
        struct AE2RingBuffer *buffer, void **pdata, size_t required_size)
{
    AE2RingBufferApiResult ret;

    /* 読み出し */
    if ((ret = AE2RingBuffer_Peek(buffer, pdata, required_size)) != AE2RINGBUFFER_APIRESULT_OK) {
        return ret;
    }

    /* バッファ参照位置更新 */
    buffer->read_pos = (buffer->read_pos + (uint32_t)required_size) % buffer->buffer_size;

    return AE2RINGBUFFER_APIRESULT_OK;
}


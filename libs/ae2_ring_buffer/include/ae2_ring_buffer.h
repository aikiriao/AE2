/*!
 * @file ae2_ring_buffer.h
 * @brief リングバッファライブラリ
 */
#ifndef AE2RINGBUFFER_H_INCLUDED
#define AE2RINGBUFFER_H_INCLUDED

#include <stdint.h>
#include <stddef.h>
#include <limits.h>

#if CHAR_BIT != 8
#error "This program run at must be CHAR_BIT == 8"
#endif

/*!
 * @brief リングバッファ生成コンフィグ 
 */
struct AE2RingBufferConfig {
    size_t max_size; /*!< バッファサイズ */
    size_t max_required_size; /*!< 取り出し最大サイズ */
};

/*!
 * @brief リングバッファAPIの結果型
 */
typedef enum AE2RingBufferApiResult {
    AE2RINGBUFFER_APIRESULT_OK = 0, /*!< 成功 */
    AE2RINGBUFFER_APIRESULT_INVALID_ARGUMENT, /*!< 不正な引数 */
    AE2RINGBUFFER_APIRESULT_EXCEED_MAX_CAPACITY, /*!< 空きサイズを超えて入力しようとした */
    AE2RINGBUFFER_APIRESULT_EXCEED_MAX_REMAIN, /*!< 残りサイズを超えて出力しようとした */
    AE2RINGBUFFER_APIRESULT_EXCEED_MAX_REQUIRED, /*!< 最大要求サイズを超えて出力しようとした */
    AE2RINGBUFFER_APIRESULT_NG /*!< その他分類不能な失敗 */
} AE2RingBufferApiResult;

/*!
 * @brief リングバッファ構造体
 */
struct AE2RingBuffer;

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/*!
 * @brief リングバッファ作成に必要なワークサイズ計算
 * @param[in] config リングバッファ生成コンフィグ
 * @return int32_t 計算に成功した場合は0以上の値を、失敗した場合は負の値を返します
 * @sa AE2RingBuffer_Create
 */
int32_t AE2RingBuffer_CalculateWorkSize(const struct AE2RingBufferConfig *config);

/*!
 * @brief リングバッファ作成
 * @param[in] config リングバッファ生成コンフィグ
 * @param[in,out] work リングバッファ生成に使用するワーク領域
 * @param[in] work_size リングバッファ生成に使用するワーク領域サイズ
 * @return AE2RingBuffer 生成に成功した場合は構造体のポインタを、失敗した場合はNULLを返します
 * @sa AE2RingBuffer_CalculateWorkSize
 */
struct AE2RingBuffer *AE2RingBuffer_Create(const struct AE2RingBufferConfig *config, void *work, int32_t work_size);

/*!
 * @brief リングバッファ破棄
 * @param[in,out] buffer リングバッファ
 * @sa AE2RingBuffer_Create
 * @attention 本関数実行後、リングバッファは不定になります
 */
void AE2RingBuffer_Destroy(struct AE2RingBuffer *buffer);

/*!
 * @brief リングバッファの内容をクリア
 * @param[in,out] buffer リングバッファ
 */
void AE2RingBuffer_Clear(struct AE2RingBuffer *buffer);

/*!
 * @brief リングバッファ内に残った（入っている）データサイズ取得
 * @param[in] buffer リングバッファ
 * @return size_t リングバッファ内に残ったデータサイズ
 */
size_t AE2RingBuffer_GetRemainSize(const struct AE2RingBuffer *buffer);

/*!
 * @brief リングバッファ内の空き（入れられる）データサイズ取得
 * @param[in] buffer リングバッファ
 * @return size_t リングバッファ内の空きデータサイズ
 */
size_t AE2RingBuffer_GetCapacitySize(const struct AE2RingBuffer *buffer);

/*!
 * @brief データ挿入
 * @param[in,out] buffer リングバッファ
 * @param[in] data 挿入データ
 * @param[in] size 挿入データサイズ
 * @return AE2RingBufferApiResult 実行結果
 */
AE2RingBufferApiResult AE2RingBuffer_Put(
        struct AE2RingBuffer *buffer, const void *data, size_t size);

/*!
 * @brief データを見る（バッファの状態は更新されない）
 * @param[in] buffer リングバッファ
 * @param[out] pdata 取り出したデータの先頭を指すポインタ
 * @param[in] required_size 取り出しデータサイズ
 * @return AE2RingBufferApiResult 実行結果
 * @attention 取り出した領域は、バッファが一周する前に使用しないと上書きされます
 */
AE2RingBufferApiResult AE2RingBuffer_Peek(
        const struct AE2RingBuffer *buffer, void **pdata, size_t required_size);

/*!
 * @brief データを取り出す
 * @param[in,out] buffer リングバッファ
 * @param[out] pdata 取り出したデータの先頭を指すポインタ
 * @param[in] required_size 取り出しデータサイズ
 * @return AE2RingBufferApiResult 実行結果
 * @attention 取り出した領域は、バッファが一周する前に使用しないと上書きされます
 */
AE2RingBufferApiResult AE2RingBuffer_Get(
        struct AE2RingBuffer *buffer, void **pdata, size_t required_size);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* AE2RINGBUFFER_H_INCLUDED */

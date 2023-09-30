/*!
* @file ae2_delay.h
* @brief デジタルディレイライブラリ
*/
#ifndef AE2DELAY_H_INCLUDED
#define AE2DELAY_H_INCLUDED

#include <stdint.h>

/*!
* @brief ディレイ生成コンフィグ
*/
struct AE2DelayConfig {
    int32_t max_num_delay_samples; /*!< 最大遅延量 */
    int32_t max_num_prodess_samples; /*!< 最大処理サンプルサイズ */
    /* TODO: タップ数 */
};

/*!
* @brief ディレイの補間タイプ
*/
typedef enum {
    AE2DELAY_FADETYPE_LINEAR = 0, /*!< 線形補間 */
    AE2DELAY_FADETYPE_SQUARE, /*!< 2乗フェード */
} AE2DelayFadeType;

/*!
* @brief ディレイ構造体
*/
struct AE2Delay;

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/*!
* @brief ディレイ作成に必要なワークサイズ計算
* @param[in] max_num_delay_samples 最大ディレイサンプル数
* @return int32_t 計算に成功した場合は0以上の値を、失敗した場合は負の値を返します
* @sa AE2Delay_Create
*/
int32_t AE2Delay_CalculateWorkSize(const struct AE2DelayConfig *config);

/*!
* @brief ディレイ作成
* @param[in] max_num_delay_samples 最大ディレイサンプル数
* @param[in,out] work ディレイ生成に使用するワーク領域
* @param[in] work_size ディレイ生成に使用するワーク領域サイズ
* @return AE2Delay 生成に成功した場合は構造体のポインタを、失敗した場合はNULLを返します
* @sa AE2Delay_CalculateWorkSize
*/
struct AE2Delay *AE2Delay_Create(const struct AE2DelayConfig *config, void *work, int32_t work_size);

/*!
* @brief ディレイ破棄
* @param[in,out] delay ディレイ
* @sa AE2Delay_Create
* @attention 本関数実行後、ディレイは不定になります
*/
void AE2Delay_Destroy(struct AE2Delay *delay);

/*!
* @brief ディレイリセット
* @param[in,out] delay ディレイ
*/
void AE2Delay_Reset(struct AE2Delay *delay);

/*!
* @brief 遅延量を設定
* @param[in,out] delay ディレイ
* @param[in] num_delay_samples 遅延サンプル数
* @param[in] fade_type フェード曲線タイプ
* @param[in] num_fade_samples フェードサンプル数（0は即時変更）
*/
void AE2Delay_SetDelay(struct AE2Delay *delay,
    int32_t num_delay_samples, AE2DelayFadeType fade_type, int32_t num_fade_samples);

/*!
* @brief 信号処理(in-place)
* @param[in,out] delay ディレイ
* @param[in,out] data 入力信号データ
* @param[in] サンプル数
*/
void AE2Delay_Process(struct AE2Delay *delay, float *data, int32_t num_samples);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* AE2DELAY_H_INCLUDED */

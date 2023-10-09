/*!
* @file ae2_simple_hrtf.h
* @brief 単純化したHRTF
*/
#ifndef AE2SIMPLEHRTF_H_INCLUDED
#define AE2SIMPLEHRTF_H_INCLUDED

#include <stdint.h>

/*! 頭部実効半径の初期値[m] */
#define AE2SIMPLEHRTF_DEFAULT_RADIUS_OF_HEAD 0.0875

/*!
* @brief HRTF生成コンフィグ
*/
struct AE2SimpleHRTFConfig {
    float sampling_rate; /*!< サンプリングレート */
    float max_radius_of_head; /*!< 最大の頭の半径 */
    float delay_fade_time_ms; /*!< 内部ディレイのフェード時間[ms] */
    int32_t max_num_process_samples; /*!< 最大処理サンプル数 */
};

/*!
* @brief HRTF構造体
*/
struct AE2SimpleHRTF;

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/*!
* @brief HRTFハンドル作成に必要なワークサイズ計算
* @param[in] config HRTF生成コンフィグ
* @return int32_t 計算に成功した場合は0以上の値を、失敗した場合は負の値を返します
* @sa AE2SimpleHRTF_Create
*/
int32_t AE2SimpleHRTF_CalculateWorkSize(const struct AE2SimpleHRTFConfig *config);

/*!
* @brief HRTFハンドル作成
* @param[in] config HRTF生成コンフィグ
* @param[in,out] work HRTF生成に使用するワーク領域
* @param[in] work_size HRTF生成に使用するワーク領域サイズ
* @return AE2SimpleHRTF 生成に成功した場合は構造体のポインタを、失敗した場合はNULLを返します
* @sa AE2SimpleHRTF_CalculateWorkSize
*/
struct AE2SimpleHRTF *AE2SimpleHRTF_Create(
    const struct AE2SimpleHRTFConfig *config, void *work, int32_t work_size);

/*!
* @brief HRTFハンドル破棄
* @param[in,out] hrtf HRTFハンドル
*/
void AE2SimpleHRTF_Destroy(struct AE2SimpleHRTF *hrtf);

/*!
* @brief HRTFハンドルリセット
* @param[in,out] hrtf HRTFハンドル
*/
void AE2SimpleHRTF_Reset(struct AE2SimpleHRTF *hrtf);

/*!
* @brief 実効頭部半径の設定
* @param[in,out] hrtf HRTFハンドル
* @param[in] radius 実効頭部半径（メートル単位）
* @sa AE2SimpleHRTF_SetAzimuthAngle
*/
void AE2SimpleHRTF_SetHeadRadius(struct AE2SimpleHRTF *hrtf, float radius);

/*!
* @brief 水平方向角度の設定
* @param[in,out] hrtf HRTFハンドル
* @param[in] angle "音源-頭部中心" と "音源-観測点" がなす角（ラジアン）
* @sa AE2SimpleHRTF_SetHeadRadius
*/
void AE2SimpleHRTF_SetAzimuthAngle(struct AE2SimpleHRTF *hrtf, float angle);

/*!
* @brief HRTF適用
* @param[in,out] hrtf HRTFハンドル
* @param[in,out] data 適用対象のデータ列
* @param[in] num_samples サンプル数
*/
void AE2SimpleHRTF_Process(
    struct AE2SimpleHRTF *hrtf, float *data, uint32_t num_samples);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* AE2SIMPLEHRTF_H_INCLUDED */

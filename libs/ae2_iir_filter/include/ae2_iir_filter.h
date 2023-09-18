/*!
* @file ae2_iir_filter.h
* @brief IIRフィルタライブラリ
*/
#ifndef AE2IIRFILTER_H_INCLUDED
#define AE2IIRFILTER_H_INCLUDED

#include <stdint.h>

/*!
* @brief IIRフィルタ
*/
struct AE2IIRFilter;

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/*!
* @brief IIRフィルタ作成に必要なワークサイズ計算
* @param[in] max_order 最大次数
* @return int32_t 計算に成功した場合は0以上の値を、失敗した場合は負の値を返します
* @sa AE2IIRFilter_Create
*/
int32_t AE2IIRFilter_CalculateWorkSize(int32_t max_order);

/*!
* @brief IIRフィルタ作成
* @param[in] max_order 最大次数
* @param[in,out] work ワーク領域
* @param[in] work_size ワーク領域サイズ
* @return int32_t 計算に成功した場合は0以上の値を、失敗した場合は負の値を返します
* @sa AE2IIRFilter_CalculateWorkSize, AE2IIRFilter_Destroy
*/
struct AE2IIRFilter *AE2IIRFilter_Create(int32_t max_order, void *work, int32_t work_size);

/*!
* @brief IIRフィルタ破棄
* @param[in,out] filter IIRフィルタ
* @sa AE2IIRFilter_CalculateWorkSize
* @attension 本関数実行後、構造体内部は不定値になります。
*/
void AE2IIRFilter_Destroy(struct AE2IIRFilter *filter);

/*!
* @brief フィルタ内部状態リセット
* @param[in,out] filter IIRフィルタ
* @attension フィルタ設定後でもフィルタ内部のバッファは不定です。信号処理開始前に本関数を実行してください。
*/
void AE2IIRFilter_ClearBuffer(struct AE2IIRFilter *filter);

/*!
* @brief フィルタ適用
* @param[in,out] filter IIRフィルタ
* @param[in,out] data 適用対象のデータ列
* @param[in] num_samples サンプル数
*/
void AE2IIRFilter_Process(
    struct AE2IIRFilter *filter, float *data, uint32_t num_samples);

/*!
* @brief ローパス特性を持つバターワースフィルタを設定
* @param[in,out] filter IIRフィルタ
* @param[in] order 次数
* @param[in] sampling_frequency サンプリングレート
* @param[in] frequency カットオフ周波数
*/
void AE2IIRFilter_SetButterworthLowPass(
    struct AE2IIRFilter *filter, int32_t order, float sampling_rate, float frequency);

/*!
* @brief ハイパス特性を持つバターワースフィルタを設定
* @param[in,out] filter IIRフィルタ
* @param[in] order 次数
* @param[in] sampling_frequency サンプリングレート
* @param[in] frequency カットオフ周波数
*/
void AE2IIRFilter_SetButterworthHighPass(
    struct AE2IIRFilter *filter, int32_t order, float sampling_rate, float frequency);

/*!
* @brief ローパス特性を持つチェビシェフフィルタを設定
* @param[in,out] filter IIRフィルタ
* @param[in] order 次数
* @param[in] sampling_frequency サンプリングレート
* @param[in] frequency カットオフ周波数
* @param[in] ripple リップルの大きさ (dB)
* @note 数値演算精度の問題上、次数orderは20程度が限度です
*/
void AE2IIRFilter_SetChebyshevLowPass(
    struct AE2IIRFilter *filter,
    int32_t order, float sampling_rate, float frequency, float ripple);

/*!
* @brief ハイパス特性を持つチェビシェフフィルタを設定
* @param[in,out] filter IIRフィルタ
* @param[in] order 次数
* @param[in] sampling_frequency サンプリングレート
* @param[in] frequency カットオフ周波数
* @param[in] ripple リップルの大きさ (dB)
* @note 数値演算精度の問題上、次数orderは20程度が限度です
*/
void AE2IIRFilter_SetChebyshevHighPass(
    struct AE2IIRFilter *filter,
    int32_t order, float sampling_rate, float frequency, float ripple);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* AE2IIRFILTER_H_INCLUDED */

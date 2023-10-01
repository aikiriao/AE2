/*!
* @file ae2_biquad_filter.h
* @brief バイクアッドフィルタライブラリ
*/
#ifndef AE2BIQUADFILTER_H_INCLUDED
#define AE2BIQUADFILTER_H_INCLUDED

#include <stdint.h>

/*!
* @brief バイクアッドフィルタ
*/
struct AE2BiquadFilter {
    float a1, a2;     /*!< フィルター伝達関数の分母（フィードバック成分）の係数 */
    float b0, b1, b2; /*!< フィルター伝達関数の分子（フィードフォワード成分）の係数 */
    float y1, y2;     /*!< 出力信号バッファ */
    float x1, x2;     /*!< 入力信号バッファ */
};

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/*!
* @brief ローパスフィルタを設定
* @param[in,out] filter バイクアッドフィルタ
* @param[in] sampling_frequency サンプリングレート
* @param[in] frequency カットオフ周波数
* @param[in] quality_factor クオリティファクタ（Q値）
* @param[in] gain ゲイン（振幅）
* @sa AE2BiquadFilter_SetHighPass, AE2BiquadFilter_SetBandPass, AE2BiquadFilter_SetBandEliminate, AE2BiquadFilter_SetAllPass, AE2BiquadFilter_SetLowShelf, AE2BiquadFilter_SetHighShelf
*/
void AE2BiquadFilter_SetLowPass(
    struct AE2BiquadFilter *filter, float sampling_rate, float frequency, float quality_factor, float gain);

/*!
* @brief ハイパスフィルタを設定
* @param[in,out] filter バイクアッドフィルタ
* @param[in] sampling_frequency サンプリングレート
* @param[in] frequency カットオフ周波数
* @param[in] quality_factor クオリティファクタ（Q値）
* @param[in] gain ゲイン（振幅）
* @sa AE2BiquadFilter_SetLowPass, AE2BiquadFilter_SetBandPass, AE2BiquadFilter_SetBandEliminate, AE2BiquadFilter_SetAllPass, AE2BiquadFilter_SetLowShelf, AE2BiquadFilter_SetHighShelf
*/
void AE2BiquadFilter_SetHighPass(
    struct AE2BiquadFilter *filter, float sampling_rate, float frequency, float quality_factor, float gain);

/*!
* @brief バンドパスフィルタを設定
* @param[in,out] filter バイクアッドフィルタ
* @param[in] sampling_frequency サンプリングレート
* @param[in] frequency 中心周波数
* @param[in] quality_factor クオリティファクタ（Q値）
* @param[in] gain ゲイン（振幅）
* @sa AE2BiquadFilter_SetLowPass, AE2BiquadFilter_SetHighPass, AE2BiquadFilter_SetBandEliminate, AE2BiquadFilter_SetAllPass, AE2BiquadFilter_SetLowShelf, AE2BiquadFilter_SetHighShelf
*/
void AE2BiquadFilter_SetBandPass(
    struct AE2BiquadFilter *filter, float sampling_rate, float frequency, float quality_factor, float gain);

/*!
* @brief バンドエリミネートフィルタを設定
* @param[in,out] filter バイクアッドフィルタ
* @param[in] sampling_frequency サンプリングレート
* @param[in] frequency 中心周波数
* @param[in] quality_factor クオリティファクタ（Q値）
* @param[in] gain ゲイン（振幅）
* @sa AE2BiquadFilter_SetLowPass, AE2BiquadFilter_SetHighPass, AE2BiquadFilter_SetBandPass, AE2BiquadFilter_SetAllPass, AE2BiquadFilter_SetLowShelf, AE2BiquadFilter_SetHighShelf
*/
void AE2BiquadFilter_SetBandEliminate(
    struct AE2BiquadFilter *filter, float sampling_rate, float frequency, float quality_factor, float gain);

/*!
* @brief オールパスフィルタを設定
* @param[in,out] filter バイクアッドフィルタ
* @param[in] sampling_frequency サンプリングレート
* @param[in] frequency 逆相（位相反転）周波数
* @param[in] quality_factor クオリティファクタ（Q値）
* @param[in] gain ゲイン（振幅）
* @sa AE2BiquadFilter_SetLowPass, AE2BiquadFilter_SetHighPass, AE2BiquadFilter_SetBandPass, AE2BiquadFilter_SetBandEliminate, AE2BiquadFilter_SetLowShelf, AE2BiquadFilter_SetHighShelf
*/
void AE2BiquadFilter_SetAllPass(
    struct AE2BiquadFilter *filter, float sampling_rate, float frequency, float quality_factor, float gain);

/*!
* @brief ローシェルフフィルタを設定
* @param[in,out] filter バイクアッドフィルタ
* @param[in] sampling_frequency サンプリングレート
* @param[in] frequency カットオフ周波数
* @param[in] quality_factor クオリティファクタ（Q値）
* @param[in] gain ゲイン（振幅）
* @sa AE2BiquadFilter_SetLowPass, AE2BiquadFilter_SetHighPass, AE2BiquadFilter_SetBandPass, AE2BiquadFilter_SetBandEliminate, AE2BiquadFilter_SetAllPass, AE2BiquadFilter_SetHighShelf
*/
void AE2BiquadFilter_SetLowShelf(
    struct AE2BiquadFilter *filter, float sampling_rate, float frequency, float quality_factor, float gain);

/*!
* @brief ハイシェルフフィルタを設定
* @param[in,out] filter バイクアッドフィルタ
* @param[in] sampling_frequency サンプリングレート
* @param[in] frequency カットオフ周波数
* @param[in] quality_factor クオリティファクタ（Q値）
* @param[in] gain ゲイン（振幅）
* @sa AE2BiquadFilter_SetLowPass, AE2BiquadFilter_SetHighPass, AE2BiquadFilter_SetBandPass, AE2BiquadFilter_SetBandEliminate, AE2BiquadFilter_SetAllPass, AE2BiquadFilter_SetLowShelf
*/
void AE2BiquadFilter_SetHighShelf(
    struct AE2BiquadFilter *filter, float sampling_rate, float frequency, float quality_factor, float gain);

/*!
* @brief フィルタ内部状態リセット
* @param[in,out] filter バイクアッドフィルタ
* @attension フィルタ設定後でもフィルタ内部のバッファは不定です。信号処理開始前に本関数を実行してください。
*/
void AE2BiquadFilter_ClearBuffer(struct AE2BiquadFilter *filter);

/*!
* @brief フィルタ適用
* @param[in,out] filter バイクアッドフィルタ
* @param[in,out] data 適用対象のデータ列
* @param[in] num_samples サンプル数
*/
void AE2BiquadFilter_Process(
        struct AE2BiquadFilter *filter, float *data, uint32_t num_samples);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* AE2BIQUADFILTER_H_INCLUDED */

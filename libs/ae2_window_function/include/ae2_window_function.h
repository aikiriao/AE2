/*!
 * @file ae2_window_function.h
 * @brief 窓関数ライブラリ
 */
#ifndef AE2WINDOWFUNCTION_H_INCLUDED
#define AE2WINDOWFUNCTION_H_INCLUDED

#include <stdint.h>

/*!
 * @brief 窓関数タイプ
 */
typedef enum {
    AE2WINDOWFUNCTION_RECTANGULAR = 0, /*!< 矩形窓 */
    AE2WINDOWFUNCTION_HANN, /*!< ハン窓 */
    AE2WINDOWFUNCTION_HAMMING,  /*!< ハミング窓 */
    AE2WINDOWFUNCTION_BLACKMAN, /*!< ブラックマン窓 */
} AE2WindowFunctionType;

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/*!
 * @brief 窓の作成
 * @param[in] type 窓関数タイプ
 * @param[in,out] window 窓を作成する領域
 * @param[in] window_size 窓のサイズ（サンプル数）
 * @note 頻繁に窓関数を適用する場合、こちらで窓を作ってから信号に乗算すると効率が良い
 * @sa AE2WindowFunction_ApplyWindow
 */
void AE2WindowFunction_MakeWindow(AE2WindowFunctionType type, float *window, uint32_t window_size);

/*!
 * @brief 窓の適用
 * @param[in] type 窓関数タイプ
 * @param[in,out] data 窓を適用する領域
 * @param[in] num_samples サンプル数
 * @sa AE2WindowFunction_MakeWindow
 */
void AE2WindowFunction_ApplyWindow(AE2WindowFunctionType type, float *data, uint32_t num_samples);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* AE2WINDOWFUNCTION_H_INCLUDED */

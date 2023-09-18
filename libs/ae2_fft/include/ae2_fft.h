/*!
* @file ae2_fft.h
* @brief FFT(Fast Fourior Transform, 高速フーリエ変換)ライブラリ
*/
#ifndef AE2FFT_H_INCLUDED
#define AE2FFT_H_INCLUDED

/*! @brief i番目の複素数の実数部にアクセス */
#define AE2FFTCOMPLEX_REAL(flt_array, i) ((flt_array)[((i) << 1)])
/*! @brief i番目の複素数の虚数部にアクセス */
#define AE2FFTCOMPLEX_IMAG(flt_array, i) ((flt_array)[((i) << 1) + 1])

#ifdef __cplusplus
extern "C" {
#endif

/*!
* @brief FFT（高速フーリエ変換）
* @param[in] n FFT点数
* @param[in] flag -1:FFT, 1:IFFT
* @param[in,out] x フーリエ変換する系列(入出力 2nサイズ必須, 偶数番目に実数部, 奇数番目に虚数部)
* @param[in,out] y 作業用配列(xと同一サイズ)
* @note 正規化は行いません
*/
void AE2FFT_FloatFFT(int n, int flag, float *x, float *y);

/*!
* @brief 実数配列のFFT（高速フーリエ変換）
* @param[in] n FFT点数
* @param[in] flag -1:FFT, 1:IFFT
* @param[in,out] x フーリエ変換する系列(入出力 nサイズ必須, FFTの場合, x[0]に直流成分の実部, x[1]に最高周波数成分の虚数部が入る)
* @param[in,out] y 作業用配列(xと同一サイズ)
* @note 正規化は行いません。正規化定数は2/nです
*/
void AE2FFT_RealFFT(int n, int flag, float *x, float *y);

#ifdef __cplusplus
}
#endif

#endif /* AE2FFT_H_INCLUDED */

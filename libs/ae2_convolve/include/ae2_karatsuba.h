#ifndef AE2KARATSUBA_H_INCLUDED
#define AE2KARATSUBA_H_INCLUDED

#include "ae2_convolve.h"

#ifdef __cplusplus
extern "C" {
#endif

/* インターフェース取得 */
const struct AE2ConvolveInterface* AE2Karatsuba_GetInterface(void);

#ifdef __cplusplus
}
#endif

#endif /* AE2KARATSUBA_H_INCLUDED */

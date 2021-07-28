#ifndef TH_BLAS_INC
#define TH_BLAS_INC

#include "THGeneral.h"

#define THBlas_(NAME) TH_CONCAT_4(TH,Real,Blas_,NAME)

#include "internal/THBlas.h"
#include "THGenerateAllTypes.h"

#endif // TH_BLAS_INC

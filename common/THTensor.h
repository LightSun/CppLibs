#ifndef TH_TENSOR_INC
#define TH_TENSOR_INC

#include "THStorage.h"
#include "THTensorApply.h"

#define THTensor          TH_CONCAT_3(TH,Real,Tensor)
#define THTensor_(NAME)   TH_CONCAT_4(TH,Real,Tensor_,NAME)

/* basics */
#include "internal/THTensor.h"
#include "THGenerateAllTypes.h"

#include "internal/THTensor.h"
#include "THGenerateHalfType.h"

#include "internal/THTensorCopy.h"
#include "THGenerateAllTypes.h"

#include "internal/THTensorCopy.h"
#include "THGenerateHalfType.h"

#include "THTensorMacros.h"

/* random numbers */
#include "THRandom.h"
#include "internal/THTensorRandom.h"
#include "THGenerateAllTypes.h"

/* maths */
#include "internal/THTensorMath.h"
#include "THGenerateAllTypes.h"

/* convolutions */
#include "internal/THTensorConv.h"
#include "THGenerateAllTypes.h"

/* lapack support */
#include "internal/THTensorLapack.h"
#include "THGenerateFloatTypes.h"

#endif

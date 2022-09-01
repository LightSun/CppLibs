#ifndef THCOMMON_H
#define THCOMMON_H

#include <stdlib.h>
#include <stdio.h>
#include <stdarg.h>
#include <math.h>
#include <limits.h>
#include <float.h>
#include <time.h>
#include <string.h>
#include <stddef.h>

#ifdef __cplusplus
# define TH_EXTERNC extern "C"
#else
# define TH_EXTERNC extern
#endif

//when lib will be called from msvc, you should use  __declspec(dllexport) to export. __declspec(dllimport) to import
#ifdef _WIN32
//# ifdef TH_EXPORTS
//#  define TH_API TH_EXTERNC __declspec(dllexport)
//# else
//#  define TH_API TH_EXTERNC __declspec(dllimport)
//# endif
# define TH_API TH_EXTERNC
#else
# define TH_API TH_EXTERNC
#endif

#ifndef M_PI
# define M_PI 3.14159265358979323846
#endif

#endif // THCOMMON_H

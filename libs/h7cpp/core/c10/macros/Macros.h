#pragma once

#ifdef _WIN32
#define C10_HIDDEN
#if defined(C10_BUILD_SHARED_LIBS)
#define C10_EXPORT __declspec(dllexport)
#define C10_IMPORT __declspec(dllimport)
#else
#define C10_EXPORT
#define C10_IMPORT
#endif
#else // _WIN32
#if defined(__GNUC__)
#define C10_EXPORT __attribute__((__visibility__("default")))
#define C10_HIDDEN __attribute__((__visibility__("hidden")))
#else // defined(__GNUC__)
#define C10_EXPORT
#define C10_HIDDEN
#endif // defined(__GNUC__)
#define C10_IMPORT C10_EXPORT
#endif // _WIN32

#ifdef NO_EXPORT
#undef C10_EXPORT
#define C10_EXPORT
#endif


//-------------
// This one is being used by libc10.so
#ifdef C10_BUILD_MAIN_LIB
#define C10_API C10_EXPORT
#else
#define C10_API C10_IMPORT
#endif

// This one is being used by libtorch.so
#ifdef CAFFE2_BUILD_MAIN_LIB
#define TORCH_API C10_EXPORT
#else
#define TORCH_API C10_IMPORT
#endif

#define C10_HOST_DEVICE

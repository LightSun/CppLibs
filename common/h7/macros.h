#ifndef MACROS_H
#define MACROS_H

#ifndef CPP_START
#ifdef __cplusplus
#define CPP_START extern "C"{
#else
#define CPP_START
#endif
#endif

#ifndef CPP_END
#ifdef __cplusplus
#define CPP_END }
#else
#define CPP_END
#endif
#endif

#ifndef H7_EXTERNC
#ifdef __cplusplus
# define H7_EXTERNC extern "C"
#else
# define H7_EXTERNC extern
#endif
#endif

#endif // MACROS_H

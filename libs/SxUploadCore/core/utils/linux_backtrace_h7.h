#ifndef __LINUX__BACKTRACE_H7_
#define __LINUX__BACKTRACE_H7_

#ifdef __cplusplus
extern "C" {
#endif

#ifdef __linux__
void _get_backtrace2(const char* exe_name);
#endif
void _get_backtrace();

#ifdef __cplusplus
}
#endif

#endif

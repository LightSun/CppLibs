#ifndef _TIMES_H
#define _TIMES_H

//ms
#if defined(_WIN32)|| defined(WIN32)
    #include <windows.h>
#define SLEEP(x) Sleep(x) //us
#else
    #include <unistd.h>
#define SLEEP(x) usleep(x*1000)
#endif

#endif // TIMES_H

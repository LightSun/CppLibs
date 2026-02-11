#pragma once


#if __x86_64__ || _M_X64
#include "fastqueue/fast_queue_x86_64.h"
#elif __aarch64__ || _M_ARM64
#include "fastqueue/fast_queue_arm64.h"
#else
#error Architecture not supported
#endif

#include "fastqueue/deaod_spsc/spsc_queue.hpp" //Deaod
#include "fastqueue/dro/spsc-queue.hpp" //Dro
#include "fastqueue/pin_thread.h"

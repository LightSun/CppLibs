/**
  callgrind. only used for linux.
  */
#ifndef CALLGRIND_HELPER_H
#define CALLGRIND_HELPER_H

#include <functional>
#include <string>

#ifdef __linux
#define USE_CALLVARIND 1
#include "callgrind.h"
#endif

namespace h7 {

struct CallgrindParams{
    int number_warmup {0};
    int repeats {0};
    void* ctx_warmup {nullptr};
    void* ctx_mainBody {nullptr};
    std::function<void()> func_setup;
    std::function<void(void*,int,int)> func_warmup;
    std::function<std::string(void*,int,int)> func_mainBody;
};
//used for trace callback.
static inline void run_with_callgrind(const CallgrindParams& ps){
#ifdef __linux
    if(ps.func_setup){
        ps.func_setup();
    }
    if(ps.func_warmup){
        const int warmupCnt = ps.number_warmup != 0 ? ps.number_warmup : 1;
        for(int i = 0 ; i < warmupCnt; ++i){
            (void)i;
            ps.func_warmup(ps.ctx_warmup, i, warmupCnt);
        }
    }
    if(ps.func_mainBody){
        const int repCnt = ps.repeats != 0 ? ps.repeats : 1;
        for(int i = 0 ; i < repCnt; ++i){
            (void)i;
            CALLGRIND_TOGGLE_COLLECT;
            auto str = ps.func_mainBody(ps.ctx_mainBody, i, repCnt);
            CALLGRIND_TOGGLE_COLLECT;
            if(str.empty()){
                CALLGRIND_DUMP_STATS;
            }else{
                CALLGRIND_DUMP_STATS_AT(str.data());
            }
        }
    }
#else
    fprintf(stderr, "only linux support callgrind.\n");
#endif
}

static inline void callgrind_collect(){
#ifdef USE_CALLVARIND
CALLGRIND_TOGGLE_COLLECT;
#endif
}

static inline void callgrind_stat(const std::string& str){
#ifdef USE_CALLVARIND
if(str.empty()){
    CALLGRIND_DUMP_STATS;
}else{
    CALLGRIND_DUMP_STATS_AT(str.data());
}
#endif
}

static inline void callgrind_begin(){
    callgrind_collect();
}

static inline void callgrind_end(const std::string& str){
    callgrind_collect();
    callgrind_stat(str);
}

}

#endif // CALLGRIND_HELPER_H

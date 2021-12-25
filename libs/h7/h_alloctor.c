
#include <stdlib.h>
#include <string.h>
#include "h_alloctor.h"

typedef void (*Free)(void*);

static void* h_alloctor_default(void* old_data, int new_size){
    if(old_data == NULL){
        return malloc(new_size);
    }
    return realloc(old_data, new_size);
}

static HAlloctor __h_alloc = h_alloctor_default;
static Free _func = free;

void h_alloctor_set(HAlloctor h_alloc){
    __h_alloc = h_alloc;
}
void h_alloctor_set_free(void (*Func)(void*)){
    _func = Func;
}

void* h_alloctor_realloc(void* old_data, int new_size){
    return __h_alloc(old_data, new_size);
}

void* h_alloctor_alloc(int new_size){
    return __h_alloc(NULL, new_size);
}
void* h_alloctor_calloc(int new_size){
    void* d = __h_alloc(NULL, new_size);
    if(d){
        memset(d, 0, new_size);
    }
    return d;
}

void h_alloctor_free(void* data){
    if(data) _func(data);
}

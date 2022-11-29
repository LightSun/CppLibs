#include "h7_alloc.h"
#include <stdlib.h>
#include <stdio.h>
#include <memory.h>
#include <string.h>

static struct core_allocator* _alloc = NULL;

void h7_set_core_allocator(struct core_allocator* alloc){
    _alloc = alloc;
}
struct core_allocator* h7_get_core_allocator(){
    if(_alloc == NULL){
        _alloc = malloc(sizeof(core_allocator));
        _alloc->Alloc = malloc;
        _alloc->Realloc = realloc;
        _alloc->Free = free;
    }
    return _alloc;
}

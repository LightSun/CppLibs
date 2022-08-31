#ifndef H7_ARRAY_MAP_H
#define H7_ARRAY_MAP_H

#include "h7_common.h"

typedef struct array_map array_map;
typedef array_map* array_map_p;

struct array_map{
    uint32 capacity;
    uint32 len_entry;
    uint16 key_ele_size;
    uint16 val_ele_size;

    struct core_allocator* ca;
    void* keys;
    void* values;
    uint32* hashes;
};

array_map_p array_map_new(struct core_allocator* ca, uint16 key_unit_size,
                          uint16 val_unit_size, uint32 init_len);
void array_map_resize(array_map_p ptr, uint32 size);

void array_map_put(array_map_p ptr, void* key, void* value, void* oldVal);
int array_map_get(array_map_p ptr, void* key, void* oldVal);
array_map_p array_map_copy(array_map_p ptr);

inline uint32 array_map_size(array_map_p ptr){
    return ptr->len_entry;
}
inline void* array_map_keys(array_map_p ptr){
    return ptr->keys;
}
inline void* array_map_values(array_map_p ptr){
    return ptr->values;
}

#define array_map_foreach(ptr, kt, code) \
    for(uint32 i = 0 ; i < ptr->len_entry ; ++i){\
        kt* key = (kt*)((char*)ptr->keys + ptr->key_ele_size * i);\
        code;\
    }

#endif

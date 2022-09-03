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
//
void array_map_prepare_size(array_map_p ptr, uint32 size);

void array_map_put(array_map_p ptr, void* key, void* value, void* oldVal);
//return 1 if success.
int array_map_get(array_map_p ptr, void* key, void* oldVal);

void* array_map_rawget(array_map_p ptr, void* key);

int array_map_remove(array_map_p ptr, void* key, void* oldVal);

array_map_p array_map_copy(array_map_p ptr);

void array_map_delete(array_map_p ptr);

#define array_map_size(ptr) ptr->len_entry
#define array_map_keys(ptr) ptr->keys
#define array_map_values(ptr) ptr->values
#define array_map_new2(ca, k_size, v_size) array_map_new(ca, k_size, v_size, 8)

#define array_map_foreach(ptr, code) \
    for(uint32 i = 0 ; i < ptr->len_entry ; ++i){\
        void* key = ((char*)ptr->keys + ptr->key_ele_size * i);\
        void* value = ((char*)ptr->values + ptr->val_ele_size * i);\
        code;\
    }

#endif

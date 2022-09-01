#include "array_map.h"
#include "h7_common.h"
#include <memory.h>
#include "hash.h"
#include "binary_search.h"
#include "mem.h"

static inline void growUpIfNeed(array_map_p ptr, uint32 delta){
    if(ptr->len_entry >= ptr->capacity - delta){
        array_map_resize(ptr, HMAX(ptr->capacity << 1, ptr->len_entry + delta));
    }
}

array_map_p array_map_new(struct core_allocator* ca, uint16 key_unit_size,
                          uint16 val_unit_size, uint32 init_len){
    array_map_p ptr = ca->Alloc(sizeof (struct array_map));
    ptr->ca = ca;
    ptr->keys = ca->Alloc(init_len * key_unit_size);
    ptr->values = ca->Alloc(init_len * val_unit_size);
    ptr->hashes = ca->Alloc(init_len * sizeof (uint32));
    ptr->capacity = init_len;
    ptr->len_entry = 0;
    ptr->key_ele_size = key_unit_size;
    ptr->val_ele_size = val_unit_size;
    return ptr;
}

void array_map_put(array_map_p ptr, void* key, void* value, void* oldVal){

    //handle hash.
    uint32 hash = fasthash32(key, ptr->key_ele_size, 0);//TODO need seed?
    int pos = binarySearch_u(ptr->hashes, 0, ptr->len_entry, hash);
    if(pos >= 0){
        // void* key_dst = (char*)ptr->keys + ptr->key_ele_size * ptr->len_entry;
        void* val_dst = (char*)ptr->values + ptr->val_ele_size * ptr->len_entry;
        //exist
        if(oldVal){
            memcpy(oldVal, val_dst, ptr->val_ele_size);
        }
        memcpy(val_dst, value, ptr->val_ele_size);
    }else{
        growUpIfNeed(ptr, 1);
        // -(pos + 1)
        int pos1 = -(pos + 1);
        arrays_insert(ptr->keys, ptr->len_entry, ptr->key_ele_size, key, pos1);
        arrays_insert(ptr->values, ptr->len_entry, ptr->val_ele_size, value, pos1);
        arrays_insert(ptr->hashes, ptr->len_entry, sizeof (uint32), &hash, pos1);
    }
    ptr->len_entry ++;
}

void array_map_resize(array_map_p ptr, uint32 size){
    ASSERT(ptr->capacity < size);
    ptr->keys = ptr->ca->Realloc(ptr->keys, ptr->capacity * ptr->key_ele_size,
                                 size * ptr->key_ele_size);
    ptr->values = ptr->ca->Realloc(ptr->values, ptr->capacity * ptr->val_ele_size,
                                 size * ptr->val_ele_size);
    ptr->hashes = ptr->ca->Realloc(ptr->hashes, ptr->capacity * sizeof (uint32),
                                 size * sizeof (uint32));
    ptr->capacity = size;
}

int array_map_get(array_map_p ptr, void* key, void* oldVal){
    ASSERT(oldVal != NULL);
    ASSERT(key != NULL);
    //handle hash.
    uint32 hash = fasthash32(key, ptr->key_ele_size, 0);//TODO need seed?
    int pos = binarySearch_u(ptr->hashes, 0, ptr->len_entry, hash);
    if(pos >= 0){
        void* val_dst = (char*)ptr->values + ptr->val_ele_size * ptr->len_entry;
        memcpy(oldVal, val_dst, ptr->val_ele_size);
        return 1;
    }
    return 0;
}

array_map_p array_map_copy(array_map_p ptr1){
    struct core_allocator* ca = ptr1->ca;
    uint32 init_len = ptr1->capacity;
    uint16 key_unit_size = ptr1->key_ele_size;
    uint16 val_unit_size = ptr1->val_ele_size;
    //
    array_map_p ptr = ca->Alloc(sizeof (struct array_map));
    ptr->ca = ca;
    ptr->keys = ca->Alloc(init_len * key_unit_size);
    ptr->values = ca->Alloc(init_len * val_unit_size);
    ptr->hashes = ca->Alloc(init_len * sizeof (uint32));
    ptr->capacity = init_len;
    ptr->len_entry = ptr1->len_entry;
    ptr->key_ele_size = key_unit_size;
    ptr->val_ele_size = val_unit_size;
    //copy data
    memcpy(ptr->keys, ptr1->keys, init_len * key_unit_size);
    memcpy(ptr->values, ptr1->values, init_len * val_unit_size);
    memcpy(ptr->hashes, ptr1->hashes, init_len * sizeof (uint32));
    return ptr;
}


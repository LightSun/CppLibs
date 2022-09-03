
#include "array_map_n.h"
#include <memory.h>
#include "hash.h"
#include "binary_search.h"
#include "mem.h"

#define DEFAULT_HASH_COUNT 8

#define prepare_key_size(ptr, delta) \
    array_map_n_prepare_keySize(ptr, delta + ptr->alloc_key_size)

#define prepare_value_size(ptr, delta)\
    array_map_n_prepare_valueSize(ptr, delta + ptr->alloc_val_size)

#define prepare_entry_count( ptr,  delta)\
    array_map_n_prepare_entryCount(ptr, ptr->hash_count + delta)

array_map_n_p array_map_n_new(struct core_allocator* ca, uint32 init_key_bytes, uint32 init_val_bytes){
    ASSERT(ca != NULL);
    array_map_n_p ptr = (array_map_n_p)ca->Alloc(sizeof (struct array_map_n));

    ptr->keys = init_key_bytes > 0 ? ca->Alloc(init_key_bytes) : NULL;
    ptr->values = init_val_bytes > 0 ? ca->Alloc(init_val_bytes) : NULL;
    ptr->key_hashes = ca->Alloc(DEFAULT_HASH_COUNT);
    ptr->key_lens = ca->Alloc(DEFAULT_HASH_COUNT);
    ptr->val_lens = ca->Alloc(DEFAULT_HASH_COUNT);
    ptr->ca = ca;

    ptr->alloc_key_size = init_key_bytes;
    ptr->alloc_val_size = init_val_bytes;
    ptr->hash_count = DEFAULT_HASH_COUNT;
    ptr->len_entry = 0;
    return ptr;
}

void array_map_n_put(array_map_n_p ptr, const void* key, uint16 key_size, const void* val, uint16 val_size,
                     struct core_mem* out){
    if(out){
        out->data = NULL;
    }
    uint32 hash = fasthash32(key, key_size, 0);
    if(ptr->len_entry == 0){
        prepare_key_size(ptr, key_size);
        prepare_value_size(ptr, val_size);
        prepare_entry_count(ptr, 1);
        memcpy(ptr->keys, key, key_size);
        memcpy(ptr->values, val, val_size);
        memcpy(ptr->key_hashes, &hash, sizeof (uint32));
        ptr->key_lens[0] = key_size;
        ptr->val_lens[0] = val_size;
        ptr->len_entry = 1;
        return;
    }
    int pos = binarySearch_uint32(ptr->key_hashes, 0, ptr->len_entry, hash);
    if(pos >= 0){
        uint16 old_val_len = ptr->val_lens[pos];
        prepare_value_size(ptr, val_size - old_val_len);
        //compute key need move size
        uint32 offsetValSize = 0;
        for(int i = 0 ; i < pos ; ++ i){
            offsetValSize += ptr->val_lens[i];
        }
        uint32 postOffsetValSize = 0;
        for(uint32 i = pos + 1 ; i < ptr->len_entry ; ++ i){
            postOffsetValSize += ptr->val_lens[i];
        }
        //copy old value if need
        if(out){
            out->ca = ptr->ca;
            out->size = old_val_len;
            out->data = ptr->ca->Alloc(old_val_len);
            memcpy(out->data, (char*)ptr->values + offsetValSize, old_val_len);
        }
        //override old value
        void* tmp_vals = ptr->ca->Alloc(postOffsetValSize);
        memcpy(tmp_vals, (char*)ptr->values + offsetValSize + old_val_len, postOffsetValSize);
        memcpy((char*)ptr->values + offsetValSize , val, val_size);
        memcpy((char*)ptr->values + offsetValSize + val_size, tmp_vals, postOffsetValSize);
        ptr->ca->Free(tmp_vals);
        ptr->val_lens[pos] = val_size;
    }else{
        prepare_key_size(ptr, key_size);
        prepare_value_size(ptr, val_size);
        prepare_entry_count(ptr, 1);
        int pos1 = - (pos +1);
        //compute key need move size
        uint32 offsetKeySize = 0;
        uint32 offsetValSize = 0;
        for(int i = 0 ; i < pos1 ; ++ i){
            offsetKeySize += ptr->key_lens[i];
            offsetValSize += ptr->val_lens[i];
        }

        uint32 postOffsetKeySize = 0;
        uint32 postOffsetValSize = 0;
        for(uint32 i = pos1 ; i < ptr->len_entry ; ++ i){
            postOffsetKeySize += ptr->key_lens[i];
            postOffsetValSize += ptr->val_lens[i];
        }
        //move keys and values which need change position
        void* tmp_keys = ptr->ca->Alloc(postOffsetKeySize);
        void* tmp_vals = ptr->ca->Alloc(postOffsetValSize);
        memcpy(tmp_keys, (char*)ptr->keys + offsetKeySize, postOffsetKeySize);
        memcpy(tmp_vals, (char*)ptr->values + offsetValSize, postOffsetValSize);
        // insert new key and value
        memcpy((char*)ptr->keys + offsetKeySize, key, key_size);
        memcpy((char*)ptr->values + offsetValSize, val, val_size);
        memcpy((char*)ptr->keys + offsetKeySize + key_size, tmp_keys, postOffsetKeySize);
        memcpy((char*)ptr->values + offsetValSize + val_size, tmp_vals, postOffsetValSize);
        //insert key and val info
        arrays_insert(ptr->key_lens, ptr->len_entry, sizeof (uint16), &key_size, pos1);
        arrays_insert(ptr->val_lens, ptr->len_entry, sizeof (uint16), &val_size, pos1);
        arrays_insert(ptr->key_hashes, ptr->len_entry, sizeof (uint32), &hash, pos1);
        //free
        ptr->ca->Free(tmp_keys);
        ptr->ca->Free(tmp_vals);
        ptr->len_entry += 1;
    }
}

void array_map_n_get(array_map_n_p ptr, const void* key, uint16 key_size,
                     struct core_mem* out){
    ASSERT(out != NULL);
    out->data = NULL;
    uint32 hash = fasthash32(key, (uint32)key_size, 0);
    int pos = binarySearch_uint32(ptr->key_hashes, 0, ptr->len_entry, hash);
    if(pos >= 0){
        uint16 old_val_len = ptr->val_lens[pos];
        out->ca = ptr->ca;
        out->size = old_val_len;
        out->data = ptr->ca->Alloc(old_val_len);
        //compute key need move size
        uint32 offsetValSize = 0;
        for(int i = 0 ; i < pos ; ++ i){
            offsetValSize += ptr->val_lens[i];
        }
        memcpy(out->data, (char*)ptr->values + offsetValSize, old_val_len);
    }
}


void array_map_n_rawget(array_map_n_p ptr, const void* key, uint16 key_size,
                        struct core_mem* out){
    ASSERT(out != NULL);
    out->data = NULL;
    uint32 hash = fasthash32(key, (uint32)key_size, 0);
    int pos = binarySearch_uint32(ptr->key_hashes, 0, ptr->len_entry, hash);
    if(pos >= 0){
        uint16 old_val_len = ptr->val_lens[pos];
        out->ca = ptr->ca;
        out->size = old_val_len;
        //compute key need move size
        uint32 offsetValSize = 0;
        for(int i = 0 ; i < pos ; ++ i){
            offsetValSize += ptr->val_lens[i];
        }
        out->data = (char*)ptr->values + offsetValSize;
    }
}

int array_map_n_remove(array_map_n_p ptr, const void* key, uint16 key_size,
                        struct core_mem* out){
    if(out){
        out->data = NULL;
    }
    uint32 hash = fasthash32(key, key_size, 0);
    int pos = binarySearch_uint32(ptr->key_hashes, 0, ptr->len_entry, hash);
    if(pos >= 0){
        uint16 old_val_len = ptr->val_lens[pos];

        //compute key need move size
        uint32 offsetValSize = 0;
        uint32 offsetKeySize = 0;
        for(int i = 0 ; i < pos ; ++ i){
            offsetValSize += ptr->val_lens[i];
            offsetKeySize += ptr->key_lens[i];
        }
        if(out){
            out->ca = ptr->ca;
            out->size = old_val_len;
            out->data = ptr->ca->Alloc(old_val_len);
            memcpy(out->data, (char*)ptr->values + offsetValSize, old_val_len);
        }
        //remove
        mem_remove(ptr->keys, ptr->alloc_key_size, offsetKeySize, ptr->key_lens[pos]);
        mem_remove(ptr->values, ptr->alloc_val_size, offsetValSize, old_val_len);
        arrays_remove(ptr->key_lens, ptr->hash_count, sizeof (uint16), pos);
        arrays_remove(ptr->val_lens, ptr->hash_count, sizeof (uint16), pos);
        arrays_remove(ptr->key_hashes, ptr->hash_count, sizeof (uint32), pos);

        ptr->len_entry --;
        return 1;
    }
    return 0;
}

array_map_n_p array_map_n_copy(array_map_n_p ptr1){
    struct core_allocator* ca = ptr1->ca;
    array_map_n_p ptr = (array_map_n_p)ca->Alloc(sizeof (struct array_map_n));

    ptr->keys = ptr1->alloc_key_size > 0 ? ca->Alloc(ptr1->alloc_key_size) : NULL;
    ptr->values = ptr1->alloc_val_size > 0 ? ca->Alloc(ptr1->alloc_val_size) : NULL;
    ptr->key_hashes = ca->Alloc(ptr1->hash_count);
    ptr->key_lens = ca->Alloc(ptr1->hash_count);
    ptr->val_lens = ca->Alloc(ptr1->hash_count);
    ptr->ca = ca;

    ptr->alloc_key_size = ptr1->alloc_key_size;
    ptr->alloc_val_size = ptr1->alloc_val_size;
    ptr->hash_count = ptr1->hash_count;
    ptr->len_entry = ptr1->len_entry;
    //copy data
    if(ptr->keys){
        memcpy(ptr->keys, ptr1->keys, ptr1->alloc_key_size);
    }
    if(ptr->keys){
        memcpy(ptr->values, ptr1->values, ptr1->alloc_val_size);
    }
    memcpy(ptr->key_hashes, ptr1->key_hashes, ptr1->hash_count * sizeof (uint32));
    memcpy(ptr->key_lens, ptr1->key_lens, ptr1->hash_count * sizeof (uint16));
    memcpy(ptr->val_lens, ptr1->val_lens, ptr1->hash_count * sizeof (uint16));
    return ptr;
}

void array_map_n_delete(array_map_n_p ptr){
    struct core_allocator* ca = ptr->ca;
    if(ptr->keys){
        ca->Free(ptr->keys);
    }
    if(ptr->values){
        ca->Free(ptr->values);
    }
    ca->Free(ptr->key_hashes);
    ca->Free(ptr->key_lens);
    ca->Free(ptr->val_lens);
    ca->Free(ptr);
}

#ifndef H7_ARRAY_MAP_N_H
#define H7_ARRAY_MAP_N_H


#include "h7/common/h7_alloc.h"

CPP_START

typedef struct array_map_n array_map_n;
typedef array_map_n* array_map_n_p;

struct array_map_n{
    struct core_allocator* ca;
    void* keys;
    void* values;
    uint32* key_hashes;
    uint16* key_lens;
    uint16* val_lens;

    uint32 alloc_key_size;
    uint32 alloc_val_size;
    uint32 len_entry;
    uint32 hash_count;
};

array_map_n_p array_map_n_new(struct core_allocator* ca, uint32 init_key_bytes, uint32 init_val_bytes);
void array_map_n_put(array_map_n_p ptr, const void* key, uint16 key_size, const void* val, uint16 val_size,
                     struct core_mem* out);

void array_map_n_get(array_map_n_p ptr, const void* key, uint16 key_size,
                     struct core_mem* out);

//raw get without copy data.
void array_map_n_rawget(array_map_n_p ptr, const void* key, uint16 key_size,
                     struct core_mem* out);

int array_map_n_remove(array_map_n_p ptr, const void* key, uint16 key_size,
                     struct core_mem* out);

array_map_n_p array_map_n_copy(array_map_n_p ptr);

void array_map_n_delete(array_map_n_p ptr);

#define array_map_n_keys(ptr) ptr->keys
#define array_map_n_values(ptr) ptr->values
#define array_map_n_keyLens(ptr) ptr->key_lens
#define array_map_n_valueLens(ptr) ptr->val_lens
#define array_map_n_size(ptr) ptr->len_entry

#define array_map_n_prepare_keySize(ptr, size)\
    if(ptr->alloc_key_size < size){\
        ptr->keys = ptr->ca->Realloc(ptr->keys, ptr->alloc_key_size, HMAX(GROWUP_HALF(ptr->alloc_key_size), size));\
        ptr->alloc_key_size = size;\
    }

#define array_map_n_prepare_valueSize(ptr, size)\
    if(ptr->alloc_val_size < size){\
        ptr->values = ptr->ca->Realloc(ptr->values, ptr->alloc_val_size, HMAX(GROWUP_HALF(ptr->alloc_val_size), size));\
        ptr->alloc_val_size = size;\
    }

#define array_map_n_prepare_entryCount(ptr, count)\
    if(ptr->hash_count < count){\
        uint32 c = HMAX(GROWUP_HALF(ptr->hash_count), count);\
        ptr->key_hashes = (uint32*)ptr->ca->Realloc(ptr->key_hashes, sizeof (uint32) * ptr->hash_count, sizeof (uint32) * c);\
        ptr->key_lens = (uint16*)ptr->ca->Realloc(ptr->key_lens, sizeof (uint16) * ptr->hash_count, sizeof (uint16) * c);\
        ptr->val_lens = (uint16*)ptr->ca->Realloc(ptr->val_lens, sizeof (uint16) * ptr->hash_count, sizeof (uint16) * c);\
        ptr->hash_count = c;\
    }


//code: used to op key and value
#define array_map_n_foreach(ptr, code) \
    do{\
        uint32 key_offset = 0;\
        uint32 val_offset = 0;\
        for(uint32 i = 0 ; i < ptr->len_entry ; ++i){\
            void* key = ((char*)ptr->keys + key_offset);\
            void* value = ((char*)ptr->values + val_offset);\
            code;\
            key_offset += ptr->key_lens[i];\
            val_offset += ptr->val_lens[i];\
        }\
    }while(0);

CPP_END

#endif

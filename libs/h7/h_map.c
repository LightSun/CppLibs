
#include <stdlib.h>
#include <math.h>
#include <assert.h>
#include "h_map.h"
#include "h_mathutils.h"
#include "h_alloctor.h"
#include "h_string.h"
#include "h_atomic.h"

#define PRIME1 0xbe1f14b1
#define PRIME2 0xb4b82e39
#define PRIME3 0xced1c241
#define null NULL
#define map_keyTable(map, type) ((type*)map->keyTable)
#define map_valueTable(map, type) ((type*)map->valueTable)

static inline int hash2(h_map* map, int h) {
    h *= PRIME2;
    return H7_SHIFT_R_LOGIC(h ^ h, map->hashShift) & map->mask;
}
static inline int hash3(h_map* map, int h) {
    h *= PRIME3;
    return H7_SHIFT_R_LOGIC(h ^ h , map->hashShift) & map->mask;
}
static void _push(h_map* map, void* insertKey, void* insertValue, int index1, void* key1,
                 int index2, void* key2, int index3, void* key3);

h_map* h_map_new(Func_Hash fun_hash, Func_KeyCompare comp_key,int initialCapacity, float loadFactor){
    assert(initialCapacity >= 0 );
    assert(loadFactor > 0 );
    //initialCapacity > 0. capacity > 1 << 30.
    int cap = h7_nextPowerOfTwo(initialCapacity);
    h_map* map = (h_map*)CALLOC(sizeof (h_map));
    map->func_hash = fun_hash;
    map->func_compKey = comp_key;
    map->capacity = cap;
    map->loadFactor = loadFactor;
    map->threshold = (int)(cap * loadFactor);
    map->mask = cap - 1;
    map->hashShift = 31 - h7_numberOfTrailingZeros(cap);
    map->stashCapacity = H7_MAX(3, (int)ceil(log(cap) * 2));
    map->pushIterations = H7_MAX(H7_MIN(cap, 8), (int)sqrt(cap) / 8);
    //k-v data
    int keyUnitSize = sizeof (void*);
    int valueUnitSize = sizeof (void*);
    map->tableLen = cap + map->stashCapacity;
    map->keyTable = CALLOC(keyUnitSize * map->tableLen);
    map->valueTable = CALLOC(valueUnitSize * map->tableLen);
    map->keyUnitSize = keyUnitSize;
    map->valueUnitSize = valueUnitSize;
    return map;
}
void h_map_delete(h_map* map, Func_Delete func_key, Func_Delete func_value){
    if(h_atomic_add(&map->ref, -1) == 1){
        //TODO delete
    }
}
void* h_map_put(h_map* map, void* key, void* value){
    int hashCode = map->func_hash(key);
    //check for key exist.
    int index1 = hashCode & map->mask;
    void* key1 = map_keyTable(map, void*)[index1];
    if (map->func_compKey(key, key1) == 0) {
        void* oldValue = map_keyTable(map, void*)[index1];
        map_valueTable(map, void*)[index1] = value;
        return oldValue;
    }

    int index2 = hash2(map, hashCode);
    void* key2 = map_keyTable(map, void*)[index2];
    if (map->func_compKey(key, key2) == 0) {
        void* oldValue = map_keyTable(map, void*)[index2];
        map_valueTable(map, void*)[index2] = value;
        return oldValue;
    }

    int index3 = hash3(map, hashCode);
    void* key3 = map_keyTable(map, void*)[index3];
    if (map->func_compKey(key, key3) == 0) {
        void* oldValue = map_keyTable(map, void*)[index3];
        map_valueTable(map, void*)[index3] = value;
        return oldValue;
    }
    // Update key in the stash.
    for (int i = map->capacity, n = i + map->stashSize; i < n; i++) {
        if (map->func_compKey(key, map_keyTable(map, void*)[i]) == 0) {
            void* oldValue = ((void**)map->valueTable)[i];
            map_valueTable(map, void*)[i] = value;
            return oldValue;
        }
    }
    // Check for empty buckets.
    if (key1 == null) {
        map_keyTable(map, void*)[index1] = key;
        map_valueTable(map, void*)[index1] = value;
        if (map->size++ >= map->threshold) h_map_resize(map, map->capacity << 1);
        return null;
    }

    if (key2 == null) {
        map_keyTable(map, void*)[index2] = key;
        map_valueTable(map, void*)[index2] = value;
        if (map->size++ >= map->threshold) h_map_resize(map, map->capacity << 1);
        return null;
    }

    if (key3 == null) {
        map_keyTable(map, void*)[index3] = key;
        map_valueTable(map, void*)[index3] = value;
        if (map->size++ >= map->threshold) h_map_resize(map, map->capacity << 1);
        return null;
    }
    _push(map, key, value, index1, key1, index2, key2, index3, key3);
    return null;
}
void h_map_ensureCapacity (h_map* map, int additionalCapacity) {
    int sizeNeeded = map->size + additionalCapacity;
    if (sizeNeeded >= map->threshold) h_map_resize(map, h7_nextPowerOfTwo((int)(sizeNeeded / map->loadFactor)));
}
void* h_map_findKey (h_map* map, Func_ValueCompare valComp, void* value, int identity) {
       void** valueTable = map->valueTable;
       void** keyTable = map->keyTable;
       if (value == null) {
           for (int i = map->capacity + map->stashSize; i-- > 0;)
               if (keyTable[i] != null && valueTable[i] == null) return keyTable[i];
       } else if (identity) {
           for (int i = map->capacity + map->stashSize; i-- > 0;)
               if (valueTable[i] == value) return keyTable[i];
       } else {
           for (int i = map->capacity + map->stashSize; i-- > 0;)
               if (valComp(value, valueTable[i]) == 0) return keyTable[i];
       }
       return null;
}
static void _putStash(h_map* map,void* key, void* value){
    if (map->stashSize == map->stashCapacity) {
        // Too many pushes occurred and the stash is full, increase the table size.
        h_map_resize(map, map->capacity << 1);
        h_map_put(map, key, value);
        return;
    }
    // Store key in the stash.
    int index = map->capacity + map->stashSize;
    map_keyTable(map, void*)[index] = key;
    map_valueTable(map, void*)[index] = value;
    map->stashSize++;
    map->size++;
}
static void _push(h_map* map, void* insertKey, void* insertValue, int index1, void* key1,
                 int index2, void* key2, int index3, void* key3){
    void** valueTable = map->valueTable;
    void** keyTable = map->keyTable;
    int mask = map->mask;
    // Push keys until an empty bucket is found.
    void* evictedKey = null;
    void* evictedValue = null;
    int i = 0, pushIterations = map->pushIterations;
    do {
        switch (h7_random(3)) {
        case 0:
            evictedKey = key1;
            evictedValue = valueTable[index1];
            keyTable[index1] = insertKey;
            valueTable[index1] = insertValue;
            break;
        case 1:
            evictedKey = key2;
            evictedValue = valueTable[index2];
            keyTable[index2] = insertKey;
            valueTable[index2] = insertValue;
            break;
        default:
            evictedKey = key3;
            evictedValue = valueTable[index3];
            keyTable[index3] = insertKey;
            valueTable[index3] = insertValue;
            break;
        }
        // If the evicted key hashes to an empty bucket, put it there and stop.
        int hashCode = map->func_hash(evictedKey);
        index1 = hashCode & mask;
        key1 = keyTable[index1];
        if (key1 == null) {
            keyTable[index1] = evictedKey;
            valueTable[index1] = evictedValue;
            if (map->size++ >= map->threshold) h_map_resize(map, map->capacity << 1);
            return;
        }

        index2 = hash2(map, hashCode);
        key2 = keyTable[index2];
        if (key2 == null) {
            keyTable[index2] = evictedKey;
            valueTable[index2] = evictedValue;
            if (map->size++ >= map->threshold) h_map_resize(map, map->capacity << 1);
            return;
        }

        index3 = hash3(map, hashCode);
        key3 = keyTable[index3];
        if (key3 == null) {
            keyTable[index3] = evictedKey;
            valueTable[index3] = evictedValue;
            if (map->size++ >= map->threshold) h_map_resize(map, map->capacity << 1);
            return;
        }

        if (++i == pushIterations) break;

        insertKey = evictedKey;
        insertValue = evictedValue;
    } while (1);
    _putStash(map, evictedKey, evictedValue);
}
/** Skips checks for existing keys. */
static void _putResize(h_map* map, void* key, void* value){
    void** valueTable = map->valueTable;
    void** keyTable = map->keyTable;
    // Check for empty buckets.
    int hashCode = map->func_hash(key);
    int index1 = hashCode & map->mask;
    void* key1 = keyTable[index1];
    if (key1 == null) {
        keyTable[index1] = key;
        valueTable[index1] = value;
        if (map->size++ >= map->threshold) h_map_resize(map, map->capacity << 1);
        return;
    }

    int index2 = hash2(map, hashCode);
    void* key2 = keyTable[index2];
    if (key2 == null) {
        keyTable[index2] = key;
        valueTable[index2] = value;
        if (map->size++ >= map->threshold) h_map_resize(map, map->capacity << 1);
        return;
    }

    int index3 = hash3(map, hashCode);
    void* key3 = keyTable[index3];
    if (key3 == null) {
        keyTable[index3] = key;
        valueTable[index3] = value;
        if (map->size++ >= map->threshold) h_map_resize(map, map->capacity << 1);
        return;
    }

    _push(map, key, value, index1, key1, index2, key2, index3, key3);
}
static void* _getStash(h_map* map, void* key) {
    void** keyTable = map->keyTable;
    for (int i = map->capacity, n = i + map->stashSize; i < n; i++)
        if (map->func_compKey(key, keyTable[i]) == 0) return map_valueTable(map, void*)[i];
    return null;
}
static void* _removeStash (h_map* map, void* key) {
    void** keyTable = map->keyTable;
    void** valueTable = map->valueTable;
    for (int i = map->capacity, n = i + map->stashSize; i < n; i++) {
        if (map->func_compKey(key, keyTable[i]) == 0) {
            void** oldValue = valueTable[i];
            h_map_removeStashIndex(map, i);
            map->size--;
            return oldValue;
        }
    }
    return null;
}
static int _containsKeyStash (h_map* map, void* key) {
    void** keyTable = map->keyTable;
    for (int i = map->capacity, n = i + map->stashSize; i < n; i++)
        if (map->func_compKey(key, keyTable[i]) == 0) return 1;
    return 0;
}
//-----------------------------------------
void h_map_resize (h_map* map, int newSize) {
    int oldEndIndex = map->capacity + map->stashSize;

    map->capacity = newSize;
    map->threshold = (int)(newSize * map->loadFactor);
    map->mask = newSize - 1;
    map->hashShift = 31 - h7_numberOfTrailingZeros(newSize);
    map->stashCapacity = H7_MAX(3, (int)ceil(log(newSize)) * 2);
    map->pushIterations = H7_MAX(H7_MIN(newSize, 8), (int)sqrt(newSize) / 8);

    void** oldKeyTable = map->keyTable;
    void** oldValueTable = map->valueTable;

    map->tableLen = newSize + map->stashCapacity;
    map->keyTable = CALLOC(map->keyUnitSize * map->tableLen);
    map->valueTable = CALLOC(map->valueUnitSize * map->tableLen);

    map->size = 0;
    map->stashSize = 0;
    for (int i = 0; i < oldEndIndex; i++) {
        void* key = oldKeyTable[i];
        if (key != null) _putResize(map, key, oldValueTable[i]);
    }
}
void* h_map_get (h_map* map, void* key) {
   int hashCode = map->func_hash(key);
   int index = hashCode & map->mask;
   void** valueTable = map->valueTable;
   void** keyTable = map->keyTable;
   if (map->func_compKey(key, keyTable[index]) != 0) {
       index = hash2(map, hashCode);
       if (map->func_compKey(key, keyTable[index]) != 0) {
           index = hash3(map, hashCode);
           if (map->func_compKey(key, keyTable[index]) != 0) return _getStash(map, key);
       }
   }
   return valueTable[index];
}
void h_map_removeStashIndex(h_map* map,int index) {
    void** valueTable = map->valueTable;
    void** keyTable = map->keyTable;
    // If the removed location was not last, move the last tuple to the removed location.
    map->stashSize--;
    int lastIndex = map->capacity + map->stashSize;
    if (index < lastIndex) {
        keyTable[index] = keyTable[lastIndex];
        valueTable[index] = valueTable[lastIndex];
        valueTable[lastIndex] = null;
    } else
        valueTable[index] = null;
}
void h_map_clear (h_map* map) {
    void** valueTable = map->valueTable;
    void** keyTable = map->keyTable;
    for (int i = map->capacity + map->stashSize; i-- > 0;) {
        keyTable[i] = null;
        valueTable[i] = null;
    }
    map->size = 0;
    map->stashSize = 0;
}
void* h_map_remove (h_map* map, void* key) {
    void** valueTable = map->valueTable;
    void** keyTable = map->keyTable;
    int hashCode = map->func_hash(key);
    int index = hashCode & map->mask;
    if (map->func_compKey(key, keyTable[index])) {
        keyTable[index] = null;
        void* oldValue = valueTable[index];
        valueTable[index] = null;
        map->size--;
        return oldValue;
    }

    index = hash2(map, hashCode);
    if (map->func_compKey(key, keyTable[index])) {
        keyTable[index] = null;
        void* oldValue = valueTable[index];
        valueTable[index] = null;
        map->size--;
        return oldValue;
    }

    index = hash3(map, hashCode);
    if (map->func_compKey(key, keyTable[index])) {
        keyTable[index] = null;
        void* oldValue = valueTable[index];
        valueTable[index] = null;
        map->size--;
        return oldValue;
    }
    return _removeStash(map, key);
}
int h_map_containsValue(h_map* map, Func_ValueCompare valComp, void* value, int identity) {
    void** keyTable = map->keyTable;
    void** valueTable = map->valueTable;
    if (value == null) {
        for (int i = map->capacity + map->stashSize; i-- > 0;)
            if (keyTable[i] != null && valueTable[i] == null) return 1;
    } else if (identity) {
        for (int i = map->capacity + map->stashSize; i-- > 0;)
            if (valueTable[i] == value) return 1;
    } else {
        for (int i = map->capacity + map->stashSize; i-- > 0;)
            if (valComp(value, valueTable[i]) == 0) return 1;
    }
    return 0;
}
int h_map_containsKey (h_map* map, void* key) {
    void** keyTable = map->keyTable;
    int hashCode = map->func_hash(key);
    int index = hashCode & map->mask;
    if (map->func_compKey(key, keyTable[index]) != 0) {
        index = hash2(map, hashCode);
        if (map->func_compKey(key, keyTable[index]) != 0) {
            index = hash3(map, hashCode);
            if (map->func_compKey(key, keyTable[index]) != 0) return _containsKeyStash(map, key);
        }
    }
    return 1;
}
void h_map_dumpString(h_map* map,Func_ToStringAdd func, struct hstring* hs){
    if (map->size == 0){
        hstring_append(hs, "{}");
        return;
    }
    hstring_append(hs, "{");
    void** keyTable = map->keyTable;
    void** valueTable = map->valueTable;
    int i = map->tableLen;
    while (i-- > 0) {
        void* key = keyTable[i];
        if (key == null) continue;
        func(hs, key);
        //hstring_append(hs, func(key));
        hstring_append(hs, "=");
        //hstring_append(valueTable[i]);
        func(hs, valueTable[i]);
        break;
    }
    while (i-- > 0) {
        void* key= keyTable[i];
        if (key == null) continue;
        hstring_append(hs, ", ");
        func(hs, key);
        hstring_append(hs, "=");
        func(hs, valueTable[i]);
    }
    hstring_append(hs, "}");
}
//-------------------- iterator ----------------
typedef struct MapIterator{
    char hasNext;
    h_map* map;
    int nextIndex, currentIndex;
    char valid : 1; //1

}MapIterator;

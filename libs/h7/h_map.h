#ifndef H_MAP_H
#define H_MAP_H

/** An unordered map. This implementation is a cuckoo hash map using 3 hashes, random walking, and a small stash for problematic
 * keys. Null keys are not allowed. Null values are allowed. No allocation is done except when growing the table size. <br>
 * <br>
 * This map performs very fast get, containsKey, and remove (typically O(1), worst case O(log(n))). Put may be a bit slower,
 * depending on hash collisions. Load factors greater than 0.91 greatly increase the chances the map will have to rehash to the
 * next higher POT size.
 *
 * @author heaven7
 * @from Nathan Sweet
*/

struct hstring;
struct h_mapIterator;

typedef int (*Func_Hash)(void* key);
typedef int (*Func_Delete)(void* k_or_v);
typedef int (*Func_Compare)(void* k1, void* k2);
typedef void (*Func_ToStringAdd)(struct hstring* hs, void* v1);
typedef void* (*Func_Copy)(void* ctx, void* data);

typedef struct h_map{
    void* keyTable;
    void* valueTable;
    int tableLen;
    int keyUnitSize;
    int valueUnitSize;

    int size;
    int capacity, stashSize;
    float loadFactor;
    int hashShift, mask, threshold;
    int stashCapacity;
    int pushIterations;
    volatile int ref;

    struct h_mapIterator* iterator;
    Func_Hash func_hash;
    Func_Compare func_compKey;
    Func_Delete func_del_key;
    Func_Delete func_del_value;
}h_map;
typedef struct h_mapEntry{
    void* key;
    void* value;
}h_mapEntry;

h_map* h_map_new(Func_Hash fun_hash, Func_Compare comp_key, int initialCapacity, float loadFactor);
void h_map_delete(h_map* map);
void h_map_set_del_funcs(h_map* map, Func_Delete func_key, Func_Delete func_value);
void h_map_ref(h_map* map);

void* h_map_put(h_map* map, void* key, void* value);
void h_map_resize(h_map* map, int newSize);
int h_map_size(h_map* map);
/** Increases the size of the backing array to acommodate the specified number of additional items. Useful before adding many
     * items to avoid multiple backing array resizes. */
void h_map_ensureCapacity (h_map* map, int additionalCapacity);
/** Returns the key for the specified value, or null if it is not in the map. Note this traverses the entire map and compares
     * every value, which may be an expensive operation.
     * @param identity If true, uses == to compare the specified value with values in the map. If false, uses
     *           {@link #equals(Object)}. */
void* h_map_findKey (h_map* map, Func_Compare valComp, void* value, int identity);

void* h_map_get(h_map* map, void* key);
void h_map_clear (h_map* map);
void* h_map_remove (h_map* map, void* key);
void h_map_removeStashIndex(h_map* map,int index);
/** Returns true(1) if the specified value is in the map. Note this traverses the entire map and compares every value, which may be
     * an expensive operation.
     * @param identity If true, uses == to compare the specified value with values in the map. If false, uses
     *     . */
int h_map_containsValue(h_map* map, Func_Compare valComp, void* value, int identity);
int h_map_containsKey (h_map* map, void* key);
void h_map_dumpString(h_map* map,Func_ToStringAdd func, struct hstring* hs);
h_map* h_map_copy (h_map* map, void* ctx, Func_Copy func_key, Func_Copy func_value);
//TODO support multi thread.
struct h_mapIterator* h_map_iterator(h_map* map);
//------------------------------------------
void h_mapIterator_delete(struct h_mapIterator* it);
void h_mapIterator_advance(struct h_mapIterator* it);
void h_mapIterator_reset(struct h_mapIterator* it);
int h_mapIterator_remove(struct h_mapIterator* it);
int h_mapIterator_hasNext(struct h_mapIterator* it);
int h_mapIterator_next(struct h_mapIterator* it,h_mapEntry* entry);
int h_mapIterator_nextKey(struct h_mapIterator* it,h_mapEntry* entry);
int h_mapIterator_nextValue(struct h_mapIterator* it,h_mapEntry* entry);

#endif

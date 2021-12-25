
/** An unordered map. This implementation is a cuckoo hash map using 3 hashes, random walking, and a small stash for problematic
 * keys. Null keys are not allowed. Null values are allowed. No allocation is done except when growing the table size. <br>
 * <br>
 * This map performs very fast get, containsKey, and remove (typically O(1), worst case O(log(n))). Put may be a bit slower,
 * depending on hash collisions. Load factors greater than 0.91 greatly increase the chances the map will have to rehash to the
 * next higher POT size.
 * @author heaven7
 * @from Nathan Sweet */
typedef struct Entry{
    void* key;
    void* value;
}Entry;

struct Entries;
struct Values;
struct Keys;

struct hstring;

typedef int (*Func_Hash)(void* key);
typedef int (*Func_Delete)(void* k_or_v);
typedef int (*Func_KeyCompare)(void* k1, void* k2);
typedef int (*Func_ValueCompare)(void* v1, void* v2);
typedef void (*Func_ToStringAdd)(struct hstring* hs, void* v1);

#define H_MAP_callback(k, v)\
typedef struct h_map_callback{\
    int (*Func_Hash)(k key);\
}h_map_callback;

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

    struct Entries* entries1;
    struct Entries* entries2;
    struct Values* values1;
    struct Values* values2;
    struct Keys* keys1;
    struct Keys* keys2;
    Func_Hash func_hash;
    Func_KeyCompare func_compKey;
}h_map;

h_map* h_map_new(Func_Hash fun_hash, Func_KeyCompare comp_key, int initialCapacity, float loadFactor);
void h_map_delete(h_map* map, Func_Delete func_key, Func_Delete func_value);
void* h_map_put(h_map* map, void* key, void* value);
void h_map_resize(h_map* map, int newSize);
/** Increases the size of the backing array to acommodate the specified number of additional items. Useful before adding many
     * items to avoid multiple backing array resizes. */
void h_map_ensureCapacity (h_map* map, int additionalCapacity);
/** Returns the key for the specified value, or null if it is not in the map. Note this traverses the entire map and compares
     * every value, which may be an expensive operation.
     * @param identity If true, uses == to compare the specified value with values in the map. If false, uses
     *           {@link #equals(Object)}. */
void* h_map_findKey (h_map* map, Func_ValueCompare valComp, void* value, int identity);

void* h_map_get(h_map* map, void* key);
void h_map_clear (h_map* map);
void* h_map_remove (h_map* map, void* key);
void h_map_removeStashIndex(h_map* map,int index);
/** Returns true(1) if the specified value is in the map. Note this traverses the entire map and compares every value, which may be
     * an expensive operation.
     * @param identity If true, uses == to compare the specified value with values in the map. If false, uses
     *     . */
int h_map_containsValue(h_map* map, Func_ValueCompare valComp, void* value, int identity);
int h_map_containsKey (h_map* map, void* key);
void h_map_dumpString(h_map* map,Func_ToStringAdd func, struct hstring* hs);


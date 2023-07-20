#ifndef BINARY_SEARCH_H
#define BINARY_SEARCH_H

//#ifdef __cplusplus
//extern "C" {
//#endif

namespace h7 {

int binarySearch(int* a, int start, int len, int key);
int binarySearch_u(unsigned int* a, int start, int len, unsigned int key);

/**
 * @brief binarySearchOffset_u
 * @param a the array
 * @param ele_size the element size of a
 * @param hash_offset the hash offset of element
 * @param start the start position
 * @param len the len of array
 * @param key the key to search
 * @return the position
 */
int binarySearchOffset_u(const void* a, int ele_size, int hash_offset,
                   int start, int len, unsigned int key);

int binarySearchOffset_u64(const void* a, int ev_size,int hash_offset,
                         int start, int len, unsigned long long key);
}


#endif // BINARY_SEARCH_H

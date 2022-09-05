#ifndef H7_MEM_H
#define H7_MEM_H
#include "h7_common.h"

CPP_START

void arrays_insert(void* arr, int already_ele_count, unsigned int ele_unit_size,
                   const void* data, unsigned int target_pos);

void arrays_remove(void* arr, int already_ele_count, unsigned int ele_unit_size,
                   unsigned int target_pos);

/**
 * @brief mem_remove remove the memory
 * @param mem  the memory addr
 * @param total_size the total size of the mem
 * @param offset the offset of memory block which will be remove
 * @param size the size of memory block to remove
 */
void mem_remove(void* mem, unsigned int total_size, unsigned int offset, unsigned int size);


/**
 * @brief mem_insert insert data to memory
 * @param mem the memory addr
 * @param total_size the total size of the mem except the data size
 * @param offset the offset of memory block which will be insert
 * @param data the new data which will be insert
 * @param size the data size
 */
void mem_insert(void* mem, unsigned int total_size, unsigned int offset, const void* data, unsigned int size);

CPP_END

#endif

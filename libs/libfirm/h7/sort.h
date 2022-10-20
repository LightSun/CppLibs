#ifndef HSORT_H
#define HSORT_H

#include "h7_common.h"

CPP_START

typedef int (*FUNC_CMP_SORT)(void* ud, int index1, int index2,
                             const void* data1, const void* data2);
/**
 * @brief sortReturnIndex: bubble sort data with compose data.
 * @param data the data addr
 * @param ele_c the element count of data.
 * @param ele_len the every element_size as bytes in data
 * @param every_offset the core data offset of compare data. uint32
 * @param outIndex the out old index. if you need.or else null.
 */
void sortReturnIndex_u32(const void* data, uint32 ele_c, uint32 ele_len, uint32 every_offset,
                     uint32* outIndex);

/**
 * @brief sortReturnIndex: bubble sort data with compose data.
 * @param data the data addr
 * @param ele_c the element count of data.
 * @param ele_len the every element_size as bytes in data
 * @param every_offset the core data offset of compare data. sint32
 * @param outIndex the out old index. if you need.or else null.
 */
void sortReturnIndex_s32(const void* data, uint32 ele_c, uint32 ele_len, uint32 every_offset,
                     uint32* outIndex);

/**
 * @brief sortReturnIndex: bubble sort data with compose data.
 * @param data the data addr
 * @param ele_c the element count of data.
 * @param ele_len the every element_size as bytes in data
 * @param every_offset the core data offset of compare data. sint32
 * @param func the compare function
 * @param ud the user data for 'func'
 * @param outIndex the out old index. if you need.or else null.
 */
void sortReturnIndex_cmp(const void* data, uint32 ele_c, uint32 ele_len,
                             uint32 every_offset, FUNC_CMP_SORT func, void* ud,
                     uint32* outIndex);


CPP_END

#endif // SORT_H

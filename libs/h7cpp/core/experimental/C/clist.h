#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define VECTOR(type) \
    typedef struct { \
        type *data; \
        size_t size; \
        size_t capacity; \
    } vector_##type; \
    \
    void vector_##type##_init(vector_##type *v) { \
        v->data = NULL; \
        v->size = 0; \
        v->capacity = 0; \
    } \
    \
    void vector_##type##_push(vector_##type *v, type value) { \
        if (v->size >= v->capacity) { \
            /* 计算新容量：首次分配 4，否则翻倍 */ \
            size_t new_cap = v->capacity ? v->capacity * 2 : 4; \
            type *new_data = (type*)realloc(v->data, new_cap * sizeof(type)); \
            if (!new_data) { \
                /* 内存分配失败，保留原有数据并退出（或自行处理） */ \
                fprintf(stderr, "vector push: realloc failed\n"); \
                exit(1); \
            } \
            v->data = new_data; \
            v->capacity = new_cap; \
        } \
        v->data[v->size++] = value; \
    } \
    \
    /* 可选：预留容量，减少频繁扩容 */ \
    void vector_##type##_reserve(vector_##type *v, size_t new_cap) { \
        if (new_cap > v->capacity) { \
            type *new_data = (type*)realloc(v->data, new_cap * sizeof(type)); \
            if (!new_data) { exit(1); } \
            v->data = new_data; \
            v->capacity = new_cap; \
        } \
    } \
    \
    /* 其他常用函数（at, size, pop, free 等）*/ \
    type vector_##type##_pop(vector_##type *v) { \
        if (v->size == 0) exit(1); \
        return v->data[--v->size]; \
    } \
    \
    type vector_##type##_at(vector_##type *v, size_t index) { \
        if (index >= v->size) exit(1); \
        return v->data[index]; \
    } \
    \
    size_t vector_##type##_size(vector_##type *v) { return v->size; } \
    \
    void vector_##type##_free(vector_##type *v) { \
        free(v->data); \
        v->data = NULL; \
        v->size = v->capacity = 0; \
    }\
    void vector_##type##_shrink_to_fit(vector_##type## *v) {\
        if (v->size < v->capacity) {\
            type *new_data = realloc(v->data, v->size * sizeof(type));\
            if (new_data || v->size == 0) {\
                v->data = new_data;\
                v->capacity = v->size;\
            }\
        }\
    }

// 生成 int 和 double 版本
//VECTOR(int)
//VECTOR(double)

//int main() {
//    vector_int vi;
//    vector_int_init(&vi);

//    // 连续 push 触发多次自动扩容
//    for (int i = 0; i < 20; i++) {
//        vector_int_push(&vi, i);
//        printf("size=%zu, capacity=%zu\n", vector_int_size(&vi), vi.capacity);
//    }

//    vector_int_free(&vi);
//    return 0;
//}

//the more please use github/STC lib

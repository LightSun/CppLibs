#ifndef H_ALLOCTOR_H
#define H_ALLOCTOR_H

typedef void* (*HAlloctor)(void* old_data, int new_size);

void h_alloctor_set(HAlloctor h_alloc);
void h_alloctor_set_free(void (*Func)(void*));

void* h_alloctor_realloc(void* old_data, int new_size);
void* h_alloctor_alloc(int new_size);
void* h_alloctor_calloc(int new_size);
void h_alloctor_free(void* data);

#define MALLOC h_alloctor_alloc
#define CMALLOC h_alloctor_calloc
#define CALLOC h_alloctor_calloc
#define FREE h_alloctor_free
#define REALLOC h_alloctor_realloc

#endif // H_ALLOCTOR_H

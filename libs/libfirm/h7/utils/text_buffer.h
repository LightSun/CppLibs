#ifndef TEXT_BUFFER_H
#define TEXT_BUFFER_H

#include "h7/common/c_common.h"

#ifndef size_t
#define size_t unsigned int
#endif

typedef struct {
    char *buf;	 /* pointer to allocated memory, can be reallocated */
    size_t capacity; /* allocated size */
    size_t pos;	 /* current position in the buffer */
} TextBuffer;

void raviX_buffer_init(TextBuffer *mb, size_t initial_size);
void raviX_buffer_resize(TextBuffer *mb, size_t new_size);
void raviX_buffer_reserve(TextBuffer *mb, size_t n);
void raviX_buffer_free(TextBuffer *mb);
static inline char *raviX_buffer_data(const TextBuffer *mb) { return mb->buf; }
static inline size_t raviX_buffer_size(const TextBuffer *mb) { return mb->capacity; }
static inline size_t raviX_buffer_len(const TextBuffer *mb) { return mb->pos; }
static inline void raviX_buffer_reset(TextBuffer *mb) { mb->pos = 0; }

/* following convert input to string before adding */
 void raviX_buffer_add_string(TextBuffer *mb, const char *str);
 void raviX_buffer_add_bytes(TextBuffer *mb, const char *str, size_t len);
 void raviX_buffer_add_fstring(TextBuffer *mb, const char *str, ...) FORMAT_ATTR(2);

/* strncpy() replacement with guaranteed 0 termination */
 void raviX_string_copy(char *buf, const char *src, size_t buflen);

#endif // TEXT_BUFFER_H

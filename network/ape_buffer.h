#ifndef __APE_BUFFER_H_
#define __APE_BUFFER_H_

#include <stdlib.h>
#include <sys/types.h>
#include <stdint.h>

typedef struct {
    unsigned char *data;

    size_t size;
    size_t used;
    
    uint32_t pos;
} buffer;

void buffer_init(buffer *b);
buffer *buffer_new(size_t size);
void buffer_delete(buffer *b);
void buffer_destroy(buffer *b);
void buffer_prepare(buffer *b, size_t size);
void buffer_append_data(buffer *b, const unsigned char *data, size_t size);
void buffer_append_char(buffer *b, const unsigned char data);
void buffer_append_string(buffer *b, const char *string);
void buffer_append_string_n(buffer *b, const char *string, size_t length);
buffer *buffer_to_buffer_utf8(buffer *b);
buffer *buffer_utf8_to_buffer(buffer *b);

#endif

// vim: ts=4 sts=4 sw=4 et


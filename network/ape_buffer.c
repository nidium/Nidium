/* GPL v2 - (c) Anthony Catel <a.catel@weelya.com> 2010 */

#include "ape_buffer.h"

#include <stdlib.h>
#include <string.h>
#include <stdio.h>

void buffer_init(buffer *b)
{
    b->data = NULL;
    b->size = 0;
    b->used = 0;
    b->pos  = 0;
}

buffer *buffer_new(size_t size)
{
    buffer *b;

    b = malloc(sizeof(*b));
    buffer_init(b);

    /* TODO: removing a malloc by making b->data[] the last struct elem */

    if ((b->size = size) > 0) {
        b->data = malloc(sizeof(char) * size);
    } else {
        b->size = 0;
    }

    return b;
}

void buffer_delete(buffer *b)
{
    if (b->data != NULL) {
        free(b->data);
    }
}

void buffer_destroy(buffer *b)
{
    if (b != NULL) {
        if (b->data != NULL) {
            free(b->data);
        }
        free(b);
    }
}

void buffer_prepare(buffer *b, size_t size)
{
    if (b->size == 0) {
        b->size = size;
        b->used = 0;
        b->data = malloc(sizeof(char) * b->size);
    } else if (b->used + size > b->size) {
        if (size == 0) {
            size = 1;
        }
        b->size += size;
        b->data = realloc(b->data, sizeof(char) * b->size);
    }
}

static void buffer_prepare_for(buffer *b, size_t size, size_t forsize)
{
    if (b->size == 0) {
        b->size = size;
        b->used = 0;
        b->data = malloc(sizeof(char) * b->size);
    } else if (b->used + forsize > b->size) {
        if (size == 0) {
            size = 1;
        }
        b->size += size;
        b->data = realloc(b->data, sizeof(char) * b->size);
    }    
}

void buffer_append_data(buffer *b, const unsigned char *data, size_t size)
{
    buffer_prepare(b, size+1);
    memcpy(b->data + b->used, data, size);
    b->data[b->used+size] = '\0';
    b->used += size;
}

void buffer_append_char(buffer *b, const unsigned char data)
{
    buffer_prepare_for(b, 2048, 1);
    b->data[b->used] = data;
    b->used++;
}

void buffer_append_string(buffer *b, const char *string)
{
    buffer_append_string_n(b, string, strlen(string));
}

void buffer_append_string_n(buffer *b, const char *string, size_t length)
{
    buffer_prepare(b, length + 1);

    memcpy(b->data + b->used, string, length + 1);
    b->used += length;
}

/* taken from PHP 5.3 */
buffer *buffer_to_buffer_utf8(buffer *b)
{
    int pos = b->used;
    unsigned char *s = b->data;
    unsigned int c;

    buffer *newb = buffer_new(b->used * 4 + 1);

    while (pos > 0) {
        c = (unsigned short)(unsigned char)*s;

        if (c < 0x80) {
            newb->data[newb->used++] = (char)c;
        } else if (c < 0x800) {
            newb->data[newb->used++] = (0xc0 | (c >> 6));
            newb->data[newb->used++] = (0x80 | (c & 0x3f));
        } else if (c < 0x10000) {
            newb->data[newb->used++] = (0xe0 | (c >> 12));
            newb->data[newb->used++] = (0xc0 | ((c >> 6) & 0x3f));
            newb->data[newb->used++] = (0x80 | (c & 0x3f));
        } else if (c < 0x200000) {
            newb->data[newb->used++] = (0xf0 | (c >> 18));
            newb->data[newb->used++] = (0xe0 | ((c >> 12) & 0x3f));
            newb->data[newb->used++] = (0xc0 | ((c >> 6) & 0x3f));
            newb->data[newb->used++] = (0x80 | (c & 0x3f));
        }
        pos--;
        s++;
    }
    newb->data[newb->used] = '\0';

    if (newb->size > newb->used+1) {
        newb->size = newb->used+1;
        newb->data = realloc(newb->data, newb->size);
    }

    return newb;
}

/* taken from PHP 5.3 (sry) */
buffer *buffer_utf8_to_buffer(buffer *b)
{
    int pos = b->used;
    unsigned char *s = b->data;
    unsigned int c;

    buffer *newb = buffer_new(b->used + 1);

    while (pos > 0) {
        c = (unsigned char)(*s);

        if (c >= 0xf0) { /* four bytes encoded, 21 bits */
            if (pos-4 >= 0) {
                c = ((s[0]&7)<<18) | ((s[1]&63)<<12) | ((s[2]&63)<<6) |
                    (s[3]&63);
            } else {
                c = '?';
            }
            s += 4;
            pos -= 4;
        } else if (c >= 0xe0) { /* three bytes encoded, 16 bits */
            if (pos-3 >= 0) {
                c = ((s[0]&63)<<12) | ((s[1]&63)<<6) | (s[2]&63);
            } else {
                c = '?';
            }
            s += 3;
            pos -= 3;
        } else if (c >= 0xc0) { /* two bytes encoded, 11 bits */
            if (pos-2 >= 0) {
                c = ((s[0]&63)<<6) | (s[1]&63);
            } else {
                c = '?';
            }
            s += 2;
            pos -= 2;
        } else {
            s++;
            pos--;
        }
        newb->data[newb->used++] = (char)(c > 0xff ? '?' : c);
    }

    newb->data[newb->used] = '\0';

    if (newb->size > newb->used+1) {
        newb->size = newb->used+1;
        newb->data = realloc(newb->data, newb->size);
    }

    return newb;
}

// vim: ts=4 sts=4 sw=4 et


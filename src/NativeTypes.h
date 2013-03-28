#ifndef nativetypes_h__
#define nativetypes_h__

#include <stdint.h>
#include <stdlib.h>

typedef struct _NMLTag {
    const char *id;
    const char *tag;
    
    struct {
        const unsigned char *data;
        size_t len;
        bool isBinary;
    } content;
} NMLTag;

#endif

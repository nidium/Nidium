/*
   Copyright 2016 Nidium Inc. All rights reserved.
   Use of this source code is governed by a MIT license
   that can be found in the LICENSE file.
*/
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

typedef struct _NativeVertex {
    float Position[3];
    float TexCoord[2];
    float Modifier[2];
} NativeVertex;

typedef struct _NativeVertices {
    NativeVertex *vertices;
    unsigned int *indices;
    unsigned int nvertices;
    unsigned int nindices;
} NativeVertices;

enum NativeContextType {
    kSkia2D_ContextType,
    kWebGL_ContextType
};

#endif


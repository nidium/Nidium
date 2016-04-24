#ifndef nml_types_h__
#define nml_types_h__

#include <stdint.h>
#include <stdlib.h>

namespace Nidium {
namespace NML {

typedef struct _NMLTag {
    const char *id;
    const char *tag;

    struct {
        const unsigned char *data;
        size_t len;
        bool isBinary;
    } content;
} NMLTag;

typedef struct _Vertex {
    float Position[3];
    float TexCoord[2];
    float Modifier[2];
} Vertex;

typedef struct _Vertices {
    Vertex *vertices;
    unsigned int *indices;
    unsigned int nvertices;
    unsigned int nindices;
} Vertices;

enum ContextType {
    kSkia2D_ContextType,
    kWebGL_ContextType
};

} // namespace NML
} // namespace Nidium

#endif


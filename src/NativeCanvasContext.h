#ifndef nativecanvascontext_h__
#define nativecanvascontext_h__

#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>

class NativeCanvas2DContext;
struct NativeRect;


class NativeCanvasContext
{
public:

    /* Explicit name used by glBindAttribLocation */
    enum {
        SH_ATTR_POSITION = 1,
        SH_ATTR_TEXCOORD = 2
    };

    struct Vertex {
        float Position[3];
        float TexCoord[2];
    };

    struct Vertices {
        int nvertices;
        Vertex *vertices;
        
        int nindices;
        int *indices;
    };

    class JSObject *jsobj;
    struct JSContext *jscx;

    enum mode {
        CONTEXT_2D,
        CONTEXT_WEBGL
    } m_Mode;

    enum shaderType {
        SHADER_FRAGMENT = 0x8B30,
        SHADER_VERTEX = 0x8B31
    };

    struct {
        uint32_t vbo[2];
        Vertices *vtx;
        uint32_t program;
    } m_GLObjects;

    bool hasShader() const {
        return (m_GLObjects.program != 0);
    }
    uint32_t getProgram() const {
        return m_GLObjects.program;
    }

    uint32_t createPassThroughProgram();

    static char *processShader(const char *content, shaderType type);
    static uint32_t compileShader(const char *data, int type);

    /*
        Create a grid of |resolution^2| points using triangle strip

        Scheme : http://dan.lecocq.us/wordpress/wp-content/uploads/2009/12/strip.png
        Details: http://en.wikipedia.org/wiki/Triangle_strip
    */
    static Vertices *buildVerticesStripe(int resolution);
    
    virtual void translate(double x, double y)=0;
    virtual void setSize(int width, int height)=0;
    virtual void setScale(double x, double y, double px=1, double py=1)=0;
    virtual void clear(uint32_t color)=0;
    virtual void flush()=0;

    virtual void composeWith(NativeCanvas2DContext *layer,
        double left, double top, double opacity,
        double zoom, const NativeRect *rclip)=0;

    void resetGLContext();

    NativeCanvasContext();
    virtual ~NativeCanvasContext();
};

#endif

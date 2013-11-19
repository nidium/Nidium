#ifndef nativecanvascontext_h__
#define nativecanvascontext_h__

#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#include <SkMatrix44.h>

class NativeCanvas2DContext;
class NativeCanvasHandler;
struct NativeRect;

class NativeCanvasContext
{
public:

    /* Explicit name used by glBindAttribLocation */
    enum {
        SH_ATTR_POSITION = 0,
        SH_ATTR_TEXCOORD = 1
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
        uint32_t vao;
        Vertices *vtx;
        uint32_t program;
        struct {
            uint32_t u_projectionMatrix;
            uint32_t u_opacity;
            uint32_t u_resolution;
            uint32_t u_position;
            uint32_t u_padding;            
        } uniforms;
    } m_GLObjects;

    /*
        Check if the context has a program installed
    */
    bool hasShader() const {
        return (m_GLObjects.program != 0);
    }

    /*
        Get the current installed program or 0
    */
    uint32_t getProgram() const {
        return m_GLObjects.program;
    }

    NativeCanvasHandler *getHandler() const {
        return this->m_Handler;
    }

    const SkMatrix44 &getMatrix() const {
        return m_Transform;
    }

    /*
        Set the appropriate OpenGL state (bind buffers, ...)
    */
    void resetGLContext();

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

    NativeCanvasContext(NativeCanvasHandler *handler);
    virtual ~NativeCanvasContext();
private:
    uint32_t createPassThroughProgram();    
protected:
    /* Hold the current matrix (model) sent to the Vertex shader */
    SkMatrix44 m_Transform;
    NativeCanvasHandler *m_Handler;
    void updateMatrix(double left, double top, int layerWidth, int layerHeight);

};

#endif

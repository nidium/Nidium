#ifndef graphics_openglheader_h__
#define graphics_openglheader_h__

#define GL_GLEXT_PROTOTYPES
#if __APPLE__
#include <OpenGL/gl.h>
#include <OpenGL/glext.h>
#else
#include <GL/gl.h>
#include <GL/glext.h>
#endif

#endif


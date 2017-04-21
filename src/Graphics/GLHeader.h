/*
   Copyright 2016 Nidium Inc. All rights reserved.
   Use of this source code is governed by a MIT license
   that can be found in the LICENSE file.
*/
#ifndef graphics_openglheader_h__
#define graphics_openglheader_h__

#if __APPLE__
#include <OpenGL/gl.h>
#include <OpenGL/glext.h>
#elif NIDIUM_OPENGLES2
#include <GLES2/gl2.h>
#include <GLES2/gl2ext.h>
#include <EGL/egl.h>
#define GL_DEPTH24_STENCIL8 GL_DEPTH24_STENCIL8_OES
#else
#include <GL/gl.h>
#include <GL/glext.h>
#endif

#endif

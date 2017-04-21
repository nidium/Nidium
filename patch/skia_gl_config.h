/*
  Copyright 2016 Nidium Inc. All rights reserved.
  Use of this source code is governed by a MIT license
  that can be found in the LICENSE file.
*/

#define GR_GL_USE_BUFFER_DATA_NULL_HINT 0
#define GR_GL_MUST_USE_VBO 1
#ifdef SK_BUILD_FOR_ANDROID
    #define GR_GL_USE_NEW_SHADER_SOURCE_SIGNATURE 1
#endif

/* nidium use per GL-call callback to reset the context */
#define GR_GL_PER_GL_FUNC_CALLBACK 1

/*
   Copyright 2016 Nidium Inc. All rights reserved.
   Use of this source code is governed by a MIT license
   that can be found in the LICENSE file.
*/
#ifndef graphics_grglangleinterface_h__
#define graphics_grglangleinterface_h__

class GrGLInterface;

namespace Nidium {
namespace Graphics {

const GrGLInterface* GrGLCreateANGLEInterface();

} // namespace Graphics
} // namespace Nidium

#endif
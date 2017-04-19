/*
   Copyright 2016 Nidium Inc. All rights reserved.
   Use of this source code is governed by a MIT license
   that can be found in the LICENSE file.
*/
#ifndef binding_jsnfs_h__
#define binding_jsnfs_h__

#include "IO/NFS.h"

#include <jsapi.h>

namespace Nidium {
namespace Binding {

class JSNFS : public Nidium::IO::NFS
{
public:
    JSNFS(JSContext *cx);
    bool writeFile(const char *name_utf8,
                   size_t name_len,
                   char *content,
                   size_t len,
                   int flags = 0);

private:
    void *buildJS(const char *data,
                  size_t len,
                  const char *filename,
                  uint32_t *outlen);
    void initRoot();
    struct
    {
        JSRuntime *rt;
        JSContext *cx;
    } m_JS;
};

#endif

} // namespace Binding
} // namespace Nidium

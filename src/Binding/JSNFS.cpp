/*
   Copyright 2016 Nidium Inc. All rights reserved.
   Use of this source code is governed by a MIT license
   that can be found in the LICENSE file.
*/
#include "Binding/JSNFS.h"

#include <stdlib.h>
#include <string.h>

#ifndef _MSC_VER
#include <strings.h>
#endif

#include "Core/Path.h"

using Nidium::Core::PtrAutoDelete;
using Nidium::Core::Path;
using Nidium::IO::NFSTree;

namespace Nidium {
namespace Binding {

JSNFS::JSNFS(JSContext *cx) : IO::NFS()
{
    m_JS.cx = cx;
    m_JS.rt = JS_GetRuntime(cx);
}

bool JSNFS::writeFile(const char *name_utf8,
                      size_t name_len,
                      char *content,
                      size_t len,
                      int flags)
{
    PtrAutoDelete<char *> path(Path::Sanitize(name_utf8, NULL), free);
    int path_len;

    if (!path.ptr()) {
        return false;
    }

    path_len = strlen(path.ptr());

    if (m_Hash.get(path.ptr())) {
        /* File already exists */
        return false;
    }

    PtrAutoDelete<char *> dir(Path::GetDir(name_utf8), free);

    if (strlen(dir.ptr())) {
        dir.ptr()[strlen(dir.ptr()) - 1] = '\0';
    }

    NFSTree *parent;

    if (strlen(dir.ptr()) == 0) {
        parent = &m_Root;
    } else {
        parent = m_Hash.get(dir.ptr());
        if (!parent || !(parent->header.flags & kNFSFileType_Dir)) {
            return false;
        }
    }

    NFSTree *newfile = new NFSTree;

    newfile->meta.content = reinterpret_cast<uint8_t *>(content);
    newfile->header.size  = len;

    newfile->filename_utf8 = static_cast<char *>(malloc(path_len + 1));
    memcpy(newfile->filename_utf8, path.ptr(), path_len);
    newfile->filename_utf8[path_len] = '\0';
    newfile->header.flags            = flags;

    /*
    if (strncasecmp(&newfile->filename_utf8[path_len - 3], CONST_STR_LEN(".js"))
        == 0) {
        uint32_t bytecode_len;
        uint8_t *bytecode;

        if ((bytecode = static_cast<uint8_t *>(this->buildJS(
                 content, len, newfile->filename_utf8, &bytecode_len)))
            == NULL) {
            delete newfile;
            return false;
        }

        newfile->meta.content = static_cast<uint8_t *>(bytecode);
        newfile->header.size  = bytecode_len;

        newfile->header.flags = flags | kNFSFileType_JSBytecode;
    }
    */

    newfile->next                   = parent->meta.children;
    newfile->header.filename_length = path_len;

    parent->header.size++;
    parent->meta.children = newfile;

    m_Hash.set(newfile->filename_utf8, newfile);
    m_Header.numfiles++;

    return true;
}

void *JSNFS::buildJS(const char *data,
                     size_t len,
                     const char *filename,
                     uint32_t *outlen)
{
    JS::RootedObject gbl(m_JS.cx, JS::CurrentGlobalOrNull(m_JS.cx));
    JS::CompileOptions options(m_JS.cx);

    options.setUTF8(true).setFileAndLine(filename, 1).setNoScriptRval(true);

    JS::RootedObject rgbl(m_JS.cx, gbl);

    bool state;

    JS::RootedScript script(m_JS.cx);

    state = JS::Compile(m_JS.cx, options, data, len, &script);

    if (!state) {
        if (JS_IsExceptionPending(m_JS.cx)) {
            if (!JS_ReportPendingException(m_JS.cx)) {
                JS_ClearPendingException(m_JS.cx);
            }
        }
        return NULL;
    }

    return JS_EncodeScript(m_JS.cx, script, outlen);
}

void JSNFS::initRoot()
{
    m_JS.cx = NULL;
    m_JS.rt = NULL;
    NFS::initRoot();
}

} // namespace Binding
} // namespace Nidium

/*
    NativeJS Core Library
    Copyright (C) 2014 Anthony Catel <paraboul@gmail.com>

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Lesser General Public
    License as published by the Free Software Foundation; either
    version 2.1 of the License, or (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Lesser General Public License for more details.

    You should have received a copy of the GNU Lesser General Public
    License along with this library; if not, write to the Free Software
    Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
*/

#include "NativeNFS.h"
#include <NativePath.h>
#include <string.h>
#include <jsapi.h>

NativeNFS::NativeNFS(uint8_t *content, size_t size) :
    m_ContentPtr(0)
{
    m_Content = content;
    m_Size    = size;

    this->initRoot();

    if (!validateArchive()) {
        return;
    }
}

NativeNFS::NativeNFS() :
    m_Content(NULL), m_ContentPtr(0), m_Size(0)
{
    this->initRoot();

    m_Header.crc32 = 0;
    m_Header.flags = 0;
    m_Header.magicnumber = NATIVE_NFS_MAGIC;
    m_Header.minversion = 100;
    m_Header.numfiles = 0;
}

void NativeNFS::initRoot()
{
    m_JS.cx = NULL;
    m_JS.rt = NULL;

    m_Root.header.filename_length = 1;
    m_Root.filename_utf8 = strdup("/");
    m_Root.next = NULL;
    m_Root.meta.children = NULL;
    m_Root.header.flags = NFS_FILE_DIR;
    m_Root.header.size = 0;

    m_Hash.set(m_Root.filename_utf8, &m_Root);
}

void NativeNFS::initJSWithCX(JSContext *cx)
{
    m_JS.cx = cx;
    m_JS.rt = JS_GetRuntime(cx);
}

#if 0
struct nativenfs_file_header_s {
    uint64_t size; // describe the number of files in case it's a directory
    uint32_t crc32;
    uint16_t flags;
    uint16_t filename_length;
    /* filename utf8 */
};
#endif

bool NativeNFS::validateArchive()
{
    if (m_Size < sizeof (struct nativenfs_header_s)) {
        return false;
    }

    this->readContent(&m_Header, sizeof(struct nativenfs_header_s));

    if (m_Header.magicnumber != NATIVE_NFS_MAGIC) {
        return false;
    }

    this->readTree(&m_Root);

    return true;
}

bool NativeNFS::mkdir(const char *name_utf8, size_t name_len)
{
    bool outsideRoot = false;

    NativePtrAutoDelete<char *> path(NativePath::sanitize(name_utf8, &outsideRoot, false));
    int path_len = strlen(path.ptr());

    if (outsideRoot) {
        return false;
    }

    if (m_Hash.get(path.ptr())) {
        return false;
    }

    NativePtrAutoDelete<char *> dir(NativePath::getDir(name_utf8));
    NativeNFSTree *parent;

    if (strlen(dir.ptr())) {
        dir.ptr()[strlen(dir.ptr())-1] = '\0';
    }

    if (strlen(dir.ptr()) == 0) {
        parent = &m_Root;
    } else {
        parent = m_Hash.get(dir.ptr());
        if (!parent || !(parent->header.flags & NFS_FILE_DIR)) {
            return false;
        }
    }

    NativeNFSTree *newdir = new NativeNFSTree;

    newdir->meta.children = NULL;

    newdir->filename_utf8 = (char *)malloc(path_len + 1);
    memcpy(newdir->filename_utf8, path.ptr(), path_len);
    newdir->filename_utf8[path_len] = '\0';

    newdir->next = parent->meta.children;
    
    newdir->header.flags = NFS_FILE_DIR;
    newdir->header.filename_length = path_len;
    newdir->header.size = 0;

    parent->header.size++;
    parent->meta.children = newdir;

    m_Hash.set(newdir->filename_utf8, newdir);
    m_Header.numfiles++;

    return true;
}

bool NativeNFS::writeFile(const char *name_utf8, size_t name_len, char *content,
        size_t len, int flags)
{

    NativePtrAutoDelete<char *> path(NativePath::sanitize(name_utf8, NULL, false));
    int path_len = strlen(path.ptr());

    if (m_Hash.get(path.ptr())) {
        /* File already exists */
        return false;
    }

    NativePtrAutoDelete<char *> dir(NativePath::getDir(name_utf8));

    if (strlen(dir.ptr())) {
        dir.ptr()[strlen(dir.ptr())-1] = '\0';
    }

    NativeNFSTree *parent;

    if (strlen(dir.ptr()) == 0) {
        parent = &m_Root;
    } else {
        parent = m_Hash.get(dir.ptr());
        if (!parent || !(parent->header.flags & NFS_FILE_DIR)) {
            return false;
        }
    }

    NativeNFSTree *newfile = new NativeNFSTree;

    newfile->meta.content = (uint8_t *)content;
    newfile->header.size = len;

    newfile->filename_utf8 = (char *)malloc(path_len + 1);
    memcpy(newfile->filename_utf8, path.ptr(), path_len);
    newfile->filename_utf8[path_len] = '\0';
    newfile->header.flags = flags;

    if (strncasecmp(&newfile->filename_utf8[path_len-3], CONST_STR_LEN(".js")) == 0) {
        uint32_t bytecode_len;
        uint8_t *bytecode;

        if ((bytecode = (uint8_t *)this->buildJS(content, len, newfile->filename_utf8, &bytecode_len)) == NULL) {
            delete newfile;
            return false;
        }

        newfile->meta.content = (uint8_t *)bytecode;
        newfile->header.size = bytecode_len;

        newfile->header.flags = flags | NFS_FILE_JSBYTECODE;
    }

    newfile->next = parent->meta.children;
    newfile->header.filename_length = path_len;

    parent->header.size++;
    parent->meta.children = newfile;

    m_Hash.set(newfile->filename_utf8, newfile);
    m_Header.numfiles++;

    return true;
}

void *NativeNFS::buildJS(const char *data, size_t len, const char *filename, uint32_t *outlen)
{
    JSObject *gbl = JS_GetGlobalObject(m_JS.cx);
    JS::CompileOptions options(m_JS.cx);

    options.setUTF8(true)
           .setFileAndLine(filename, 1);

    js::RootedObject rgbl(m_JS.cx, gbl);

    JS_SetOptions(m_JS.cx, JSOPTION_VAROBJFIX|JSOPTION_NO_SCRIPT_RVAL);

    JSScript *script = JS::Compile(m_JS.cx, rgbl, options, data, len);

    if (!script) {
        if (JS_IsExceptionPending(m_JS.cx)) {
            if (!JS_ReportPendingException(m_JS.cx)) {
                JS_ClearPendingException(m_JS.cx);
            }
        }
        return NULL;
    }

    return JS_EncodeScript(m_JS.cx, script, outlen);
}

const char *NativeNFS::readFile(const char *filename, size_t *len,
    int *flags) const
{
    NativeNFSTree *file = m_Hash.get(filename);
    if (file == NULL || (file->header.flags & NFS_FILE_DIR)) {
        return NULL;
    }

    if (flags) {
        *flags = file->header.flags;
    }

    *len = file->header.size;

    return (const char *)file->meta.content;
}

bool NativeNFS::save(FILE *fd)
{
    if (!fd) {
        return false;
    }

    fwrite(&m_Header, sizeof(struct nativenfs_header_s), 1, fd);

    writeTree(fd, m_Root.meta.children);

    return true;
}

bool NativeNFS::save(const char *dest)
{
    FILE *fd = fopen(dest, "w+");
    if (!fd) {
        return false;
    }

    bool ret = this->save(fd);

    fclose(fd);

    return ret;
}

void NativeNFS::writeTree(FILE *fd, NativeNFSTree *cur)
{
    if (cur == NULL) {
        return;
    }

    fwrite(&cur->header, sizeof(struct nativenfs_file_header_s), 1, fd);
    fwrite(cur->filename_utf8, sizeof(char), cur->header.filename_length, fd);

    if (!(cur->header.flags & NFS_FILE_DIR)) {
        fwrite(cur->meta.content, 1, cur->header.size, fd);
    } else if (cur->header.flags & NFS_FILE_DIR) {
        this->writeTree(fd, cur->meta.children);
    }

    this->writeTree(fd, cur->next);
}

void NativeNFS::readTree(NativeNFSTree *parent)
{
    if (m_ContentPtr >= m_Size) {
        return;
    }

    NativeNFSTree *item = new NativeNFSTree;

    item->next = parent->meta.children;
    parent->meta.children = item;

    this->readContent(&item->header, sizeof(struct nativenfs_file_header_s));
    item->filename_utf8 = (char *)malloc(item->header.filename_length + 1);
    this->readContent(item->filename_utf8, item->header.filename_length);
    item->filename_utf8[item->header.filename_length] = '\0';

    m_Hash.set(item->filename_utf8, item);

    if (!(item->header.flags & NFS_FILE_DIR)) {
        item->meta.content = m_Content + m_ContentPtr;
        m_ContentPtr += item->header.size;
    } else if (item->header.flags & NFS_FILE_DIR) {
        this->readTree(item);
    }

    this->readTree(parent);
}

bool NativeNFS::readContent(void *dest, size_t len)
{
    if (m_ContentPtr + len > m_Size) {
        return false;
    }

    memcpy(dest, m_Content + m_ContentPtr, len);

    m_ContentPtr += len;

    return true;
}

void NativeNFS::releaseTree(NativeNFSTree *root)
{
    if (root == NULL) {
        return;
    }

    this->releaseTree(root->next);

    if (root->header.flags & NFS_FILE_DIR) {
        this->releaseTree(root->meta.children);
    }
    
    free(root->filename_utf8);
    free(root);
}

NativeNFS::~NativeNFS()
{
    free(m_Root.filename_utf8);

    this->releaseTree(m_Root.meta.children);
}

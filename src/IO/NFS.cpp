/*
   Copyright 2016 Nidium Inc. All rights reserved.
   Use of this source code is governed by a MIT license
   that can be found in the LICENSE file.
*/
#include "IO/NFS.h"

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>

#include "Core/Path.h"

using Nidium::Core::PtrAutoDelete;
using Nidium::Core::Path;

namespace Nidium {
namespace IO {

NFS::NFS(uint8_t *content, size_t size) : m_ContentPtr(0)
{
    m_Content = content;
    m_Size    = size;

    this->initRoot();

    if (!validateArchive()) {
        return;
    }
}

NFS::NFS() : m_Content(NULL), m_ContentPtr(0), m_Size(0)
{
    this->initRoot();

    m_Header.crc32       = 0;
    m_Header.flags       = 0;
    m_Header.magicnumber = NIDIUM_NFS_MAGIC;
    m_Header.minversion  = 100;
    m_Header.numfiles    = 0;
}

void NFS::initRoot()
{
    m_Root.header.filename_length = 1;
    m_Root.filename_utf8          = strdup("/");
    m_Root.next                   = NULL;
    m_Root.meta.children          = NULL;
    m_Root.header.flags           = kNFSFileType_Dir;
    m_Root.header.size            = 0;

    m_Hash.set(m_Root.filename_utf8, &m_Root);
}

#if 0
struct nfs_file_header_s {
    uint64_t size; // describe the number of files in case it's a directory
    uint32_t crc32;
    uint16_t flags;
    uint16_t filename_length;
    /* filename utf8 */
};
#endif

bool NFS::validateArchive()
{
    if (m_Size < sizeof(struct nfs_header_s)) {
        return false;
    }

    this->readContent(&m_Header, sizeof(struct nfs_header_s));

    if (m_Header.magicnumber != NIDIUM_NFS_MAGIC) {
        return false;
    }

    this->readTree(&m_Root);

    return true;
}

bool NFS::mkdir(const char *name_utf8, size_t name_len)
{
    bool outsideRoot = false;
    PtrAutoDelete<char *> path(Path::Sanitize(name_utf8, &outsideRoot));
    if (!path.ptr()) {
        return false;
    }

    int path_len = strlen(path.ptr());

    if (outsideRoot) {
        return false;
    }

    if (m_Hash.get(path.ptr())) {
        return false;
    }

    PtrAutoDelete<char *> dir(Path::GetDir(name_utf8));
    NFSTree *parent;

    if (strlen(dir.ptr())) {
        dir.ptr()[strlen(dir.ptr()) - 1] = '\0';
    }

    if (strlen(dir.ptr()) == 0) {
        parent = &m_Root;
    } else {
        parent = m_Hash.get(dir.ptr());
        if (!parent || !(parent->header.flags & kNFSFileType_Dir)) {
            return false;
        }
    }

    NFSTree *newdir = new NFSTree;

    newdir->meta.children = NULL;

    newdir->filename_utf8 = static_cast<char *>(malloc(path_len + 1));
    memcpy(newdir->filename_utf8, path.ptr(), path_len);
    newdir->filename_utf8[path_len] = '\0';

    newdir->next = parent->meta.children;

    newdir->header.flags           = kNFSFileType_Dir;
    newdir->header.filename_length = path_len;
    newdir->header.size            = 0;

    parent->header.size++;
    parent->meta.children = newdir;

    m_Hash.set(newdir->filename_utf8, newdir);
    m_Header.numfiles++;

    return true;
}

bool NFS::writeFile(const char *name_utf8,
                    size_t name_len,
                    char *content,
                    size_t len,
                    int flags)
{

    PtrAutoDelete<char *> path(Path::Sanitize(name_utf8));
    int path_len = strlen(path.ptr());

    if (m_Hash.get(path.ptr())) {
        /* File already exists */
        return false;
    }

    PtrAutoDelete<char *> dir(Path::GetDir(name_utf8));

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

    newfile->next                   = parent->meta.children;
    newfile->header.filename_length = path_len;

    parent->header.size++;
    parent->meta.children = newfile;

    m_Hash.set(newfile->filename_utf8, newfile);
    m_Header.numfiles++;

    return true;
}

int NFS::exists(const char *filename)
{
    NFSTree *file = m_Hash.get(filename);

    if (file == NULL) {
        return 0;
    }

    return (file->header.flags & kNFSFileType_Dir) ? 2 : 1;
}

const char *NFS::readFile(const char *filename, size_t *len, int *flags) const
{
    NFSTree *file = m_Hash.get(filename);
    if (file == NULL || (file->header.flags & kNFSFileType_Dir)) {
        return NULL;
    }

    if (flags) {
        *flags = file->header.flags;
    }

    *len = file->header.size;

    return reinterpret_cast<const char *>(file->meta.content);
}

bool NFS::save(FILE *fd)
{
    if (!fd) {
        return false;
    }

    fwrite(&m_Header, sizeof(struct nfs_header_s), 1, fd);

    writeTree(fd, m_Root.meta.children);

    return true;
}

bool NFS::save(const char *dest)
{
    FILE *fd = fopen(dest, "w+");
    if (!fd) {
        return false;
    }

    bool ret = this->save(fd);

    fclose(fd);

    return ret;
}

void NFS::writeTree(FILE *fd, NFSTree *cur)
{
    if (cur == NULL) {
        return;
    }

    fwrite(&cur->header, sizeof(struct nfs_file_header_s), 1, fd);
    fwrite(cur->filename_utf8, sizeof(char), cur->header.filename_length, fd);

    if (!(cur->header.flags & kNFSFileType_Dir)) {
        fwrite(cur->meta.content, 1, cur->header.size, fd);
    } else if (cur->header.flags & kNFSFileType_Dir) {
        this->writeTree(fd, cur->meta.children);
    }

    this->writeTree(fd, cur->next);
}

void NFS::readTree(NFSTree *parent)
{
    if (m_ContentPtr >= m_Size) {
        return;
    }

    NFSTree *item = new NFSTree;

    item->next            = parent->meta.children;
    parent->meta.children = item;

    this->readContent(&item->header, sizeof(struct nfs_file_header_s));
    item->filename_utf8
        = static_cast<char *>(malloc(item->header.filename_length + 1));
    this->readContent(item->filename_utf8, item->header.filename_length);
    item->filename_utf8[item->header.filename_length] = '\0';

    m_Hash.set(item->filename_utf8, item);

    if (!(item->header.flags & kNFSFileType_Dir)) {
        item->meta.content = m_Content + m_ContentPtr;
        m_ContentPtr += item->header.size;
    } else if (item->header.flags & kNFSFileType_Dir) {
        this->readTree(item);
    }

    this->readTree(parent);
}

bool NFS::readContent(void *dest, size_t len)
{
    if (m_ContentPtr + len > m_Size) {
        return false;
    }

    memcpy(dest, m_Content + m_ContentPtr, len);

    m_ContentPtr += len;

    return true;
}

void NFS::releaseTree(NFSTree *root)
{
    if (root == NULL) {
        return;
    }

    this->releaseTree(root->next);

    if (root->header.flags & kNFSFileType_Dir) {
        this->releaseTree(root->meta.children);
    }

    free(root->filename_utf8);
    free(root);
}

NFS::~NFS()
{
    free(m_Root.filename_utf8);

    this->releaseTree(m_Root.meta.children);
}

} // namespace IO
} // namespace Nidium

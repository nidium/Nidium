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

NativeNFS::NativeNFS(uint8_t *content, size_t size)
{
    m_Content = content;
    m_Size    = size;

    m_Root.header.filename_length = 1;
    m_Root.filename_utf8 = strdup("/");
    m_Root.next = NULL;
    m_Root.meta.children = NULL;
    m_Root.header.flags = NFS_FILE_DIR;
    m_Root.header.size = 0;

    if (!validateArchive()) {
        return;
    }

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

    memcpy(&m_Header, m_Content, sizeof(struct nativenfs_header_s));

    return true;
}

bool NativeNFS::mkdir(const char *name_utf8, size_t name_len)
{
    bool outsideRoot = false;

    NativePtrAutoDelete<char *> path(NativePath::sanitize(name_utf8, &outsideRoot));
    int path_len = strlen(path.ptr());

    if (outsideRoot) {
        return false;
    }

    if (m_Hash.get(path.ptr())) {
        return false;
    }

    NativePtrAutoDelete<char *> dir(NativePath::getDir(name_utf8));
    NativeNFSTree *parent;

    if (strlen(dir.ptr()) == 0) {
        parent = &m_Root;
    } else {
        parent = m_Hash.get(dir.ptr());
        if (!parent) {
            return false;
        }
    }

    NativeNFSTree *newdir = new NativeNFSTree;

    newdir->filename_utf8 = (char *)malloc(name_len + 1);
    memcpy(newdir->filename_utf8, path.ptr(), path_len);
    newdir->filename_utf8[path_len] = '\0';

    newdir->next = parent->meta.children;
    
    newdir->header.flags = NFS_FILE_DIR;
    newdir->header.filename_length = path_len;
    newdir->header.size = 0;

    parent->header.size++;
    parent->meta.children = newdir;

    return true;
}

void NativeNFS::releaseTree(NativeNFSTree *root)
{
    NativeNFSTree *cur;

    for (cur = root; cur != NULL; cur = cur->next) {
        releaseTree(cur->next);
    }

    if (root->header.flags & NFS_FILE_DIR) {
        releaseTree(root->meta.children);
    }
    
    free(root->filename_utf8);
    free(root);
}

NativeNFS::~NativeNFS()
{
    free(m_Root.filename_utf8);

    this->releaseTree(m_Root.meta.children);
}
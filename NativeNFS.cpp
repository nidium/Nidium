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

    m_Hash.set(m_Root.filename_utf8, &m_Root);

    if (!validateArchive()) {
        return;
    }
}

NativeNFS::NativeNFS() :
    m_Content(NULL), m_Size(0)
{
    m_Root.header.filename_length = 1;
    m_Root.filename_utf8 = strdup("/");
    m_Root.next = NULL;
    m_Root.meta.children = NULL;
    m_Root.header.flags = NFS_FILE_DIR;
    m_Root.header.size = 0;

    m_Hash.set(m_Root.filename_utf8, &m_Root);

    m_Header.crc32 = 0;
    m_Header.flags = 0;
    m_Header.magicnumber = NATIVE_NFS_MAGIC;
    m_Header.minversion = 100;
    m_Header.numfiles = 0;

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

    printf("Looking for %s\n", dir.ptr());

    if (strlen(dir.ptr()) == 0) {
        parent = &m_Root;
    } else {
        parent = m_Hash.get(dir.ptr());
        if (!parent || !(parent->header.flags & NFS_FILE_DIR)) {
            printf("nop %p\n", parent);
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

    newfile->filename_utf8 = (char *)malloc(path_len + 1);
    memcpy(newfile->filename_utf8, path.ptr(), path_len);
    newfile->filename_utf8[path_len] = '\0';

    newfile->next = parent->meta.children;
    
    newfile->header.flags = flags;
    newfile->header.filename_length = path_len;
    newfile->header.size = len;

    parent->header.size++;
    parent->meta.children = newfile;

    m_Hash.set(newfile->filename_utf8, newfile);
    m_Header.numfiles++;

    return true;
}

bool NativeNFS::save(const char *dest)
{
    FILE *fd = fopen(dest, "w+");
    if (!dest) {
        return false;
    }

    fwrite(&m_Header, sizeof(struct nativenfs_header_s), 1, fd);

    writeTree(fd, m_Root.meta.children);

    fclose(fd);

    return true;
}

void NativeNFS::writeTree(FILE *fd, NativeNFSTree *cur)
{
    if (cur == NULL) {
        return;
    }

    fwrite(&cur->header, sizeof(struct nativenfs_file_header_s), 1, fd);
    fwrite(cur->filename_utf8, sizeof(char), cur->header.filename_length, fd);

    printf("Writting %s\n", cur->filename_utf8);

    if (!(cur->header.flags & NFS_FILE_DIR)) {
        fwrite(cur->meta.content, 1, cur->header.size, fd);
    } else if (cur->header.flags & NFS_FILE_DIR) {
        writeTree(fd, cur->meta.children);
    }

    writeTree(fd, cur->next);
}

void NativeNFS::releaseTree(NativeNFSTree *root)
{
    NativeNFSTree *cur;

    if (root == NULL) {
        return;
    }

    releaseTree(cur->next);

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
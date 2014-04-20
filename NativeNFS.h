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

#ifndef nativenfs_h__
#define nativenfs_h__

#include <stdlib.h>
#include <stdint.h>
#include <stdio.h>

#include <NativeHash.h>
#include <jspubtd.h>

#define NATIVE_NFS_MAGIC 0x27121986

const uint16_t _native_nfs_version = 100;

struct nativenfs_header_s {
    uint32_t magicnumber;
    uint16_t minversion;
    uint16_t flags;
    uint32_t numfiles;
    uint32_t crc32;
};

struct nativenfs_file_header_s {
    uint64_t size; // describe the number of files in case it's a directory
    uint32_t crc32;
    uint16_t flags;
    uint16_t filename_length;
    /* filename utf8 */
};

typedef struct nativenfs_tree_s {
    struct nativenfs_file_header_s header;
    char *filename_utf8;

    struct nativenfs_tree_s *next;

    union { /* file : content / dir : children (defined by flags in header) */
        struct nativenfs_tree_s *children;
        uint8_t *content;
    } meta;
} NativeNFSTree;

class NativeNFS
{
public:
    enum fileType {
        NFS_FILE_BINARY         = 1 << 0,
        NFS_FILE_JSBYTECODE     = 1 << 1,
        NFS_FILE_TEXT           = 1 << 2,
        NFS_FILE_DIR            = 1 << 3
    };

    bool validateArchive();

    NativeNFS(uint8_t *content, size_t size);
    NativeNFS();
    ~NativeNFS();

    bool save(const char *dest);
    bool save(FILE *fd);

    bool mkdir(const char *name_utf8, size_t name_len);
    bool writeFile(const char *name_utf8, size_t name_len, char *content,
        size_t len, int flags = 0);

    void initJSWithCX(JSContext *cx);

    const char *readFile(const char *filename, size_t *len,
        int *flags = NULL) const;

private:
    uint8_t *m_Content;
    off_t m_ContentPtr;
    size_t m_Size;

    struct nativenfs_header_s m_Header;

    NativeHash<NativeNFSTree *> m_Hash;

    NativeNFSTree m_Root;

    void writeTree(FILE *fd, NativeNFSTree *cur);
    void readTree(NativeNFSTree *parent);
    void releaseTree(NativeNFSTree *root);

    bool readContent(void *dest, size_t len);

    void initRoot();

    void *buildJS(const char *data, size_t len, const char *filename, uint32_t *outlen);
    
    struct {
        JSRuntime *rt;
        JSContext *cx;
    } m_JS;
};


#endif
/*
   Copyright 2016 Nidium Inc. All rights reserved.
   Use of this source code is governed by a MIT license
   that can be found in the LICENSE file.
*/
#ifndef nativenfs_h__
#define nativenfs_h__

#include <stdlib.h>
#include <stdint.h>
#include <stdio.h>

#include <jspubtd.h>

#include "Core/NativeHash.h"

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


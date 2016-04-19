/*
   Copyright 2016 Nidium Inc. All rights reserved.
   Use of this source code is governed by a MIT license
   that can be found in the LICENSE file.
*/
#ifndef io_nfs_h__
#define io_nfs_h__

#include <stdlib.h>
#include <stdint.h>
#include <stdio.h>

#include <jspubtd.h>

#include "Core/Hash.h"

namespace Nidium {
namespace IO {

#define NIDIUM_NFS_MAGIC 0x27121986

const uint16_t _nfs_version = 100;

struct nfs_header_s {
    uint32_t magicnumber;
    uint16_t minversion;
    uint16_t flags;
    uint32_t numfiles;
    uint32_t crc32;
};

struct nfs_file_header_s {
    uint64_t size; // describe the number of files in case it's a directory
    uint32_t crc32;
    uint16_t flags;
    uint16_t filename_length;
    /* filename utf8 */
};

typedef struct nfs_tree_s {
    struct nfs_file_header_s header;
    char *filename_utf8;

    struct nfs_tree_s *next;

    union { /* file : content / dir : children (defined by flags in header) */
        struct nfs_tree_s *children;
        uint8_t *content;
    } meta;
} NFSTree;

class NFS
{
public:
    enum fileType {
        NFS_FILE_BINARY         = 1 << 0,
        NFS_FILE_JSBYTECODE     = 1 << 1,
        NFS_FILE_TEXT           = 1 << 2,
        NFS_FILE_DIR            = 1 << 3
    };

    bool validateArchive();

    NFS(uint8_t *content, size_t size);
    NFS();
    ~NFS();

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

    struct nfs_header_s m_Header;

    Nidium::Core::Hash<NFSTree *> m_Hash;

    NFSTree m_Root;

    void writeTree(FILE *fd, NFSTree *cur);
    void readTree(NFSTree *parent);
    void releaseTree(NFSTree *root);

    bool readContent(void *dest, size_t len);

    void initRoot();

    void *buildJS(const char *data, size_t len, const char *filename, uint32_t *outlen);

    struct {
        JSRuntime *rt;
        JSContext *cx;
    } m_JS;
};

} // namespace IO
} // namespace Nidium

#endif


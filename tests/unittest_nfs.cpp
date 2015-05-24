#include <stdlib.h>
#include <string.h>

#include "unittest.h"

#include <NativeNFS.h>


TEST(NativeNFS, Simple)
{
    class NativeNFS * nfs = new NativeNFS();
    EXPECT_TRUE(nfs != NULL);
    EXPECT_TRUE(nfs->validateArchive() == false);

    delete nfs;
}


//@TODO:    bool validateArchive();
//@TODO:    NativeNFS(uint8_t *content, size_t size);
//@TODO:    bool save(const char *dest);
//@TODO:    bool save(FILE *fd);
//@TODO:    bool mkdir(const char *name_utf8, size_t name_len);
//@TODO:    bool writeFile(const char *name_utf8, size_t name_len, char *content, size_t len, int flags = 0);
//@TODO:    void initJSWithCX(JSContext *cx);
//@TODO:    const char *readFile(const char *filename, size_t *len, int *flags = NULL) const;


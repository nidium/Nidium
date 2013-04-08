#ifndef nativenml_h__
#define nativenml_h__

#include "NativeFileIO.h"
#include <native_netlib.h>
#include "NativeAssets.h"
#include "external/rapidxml.hpp"
#include "NativeTypes.h"

class NativeNML;
class NativeJS;

typedef void (NativeNML::*tag_callback)(rapidxml::xml_node<> &node);

class NativeNML : public NativeFileIODelegate
{
    public:

    NativeNML(ape_global *net);
    ~NativeNML();
    void loadFile(const char *filename);
    void loadData(char *data, size_t len);
    void loadAssets(rapidxml::xml_node<> &node);
    void onAssetsItemReady(NativeAssets::Item *item);

    void onNFIOOpen(NativeFileIO *);
    void onNFIOError(NativeFileIO *, int errno){};
    void onNFIORead(NativeFileIO *, unsigned char *data, size_t len);
    void onNFIOWrite(NativeFileIO *, size_t written){};

    void setNJS(NativeJS *js) {
        this->njs = js;
    }

    private:

        void addAsset(NativeAssets *);
        ape_global *net;
        NativeFileIO *NFIO;

        /* Define callbacks for tags in <application> */
        struct _nml_tags {
            const char *str;
            /* Call : (this->*cb)() */
            tag_callback cb;
        } nml_tags[2] = {
            {"assets",   &NativeNML::loadAssets},
            {NULL,       NULL}
        };

        NativeJS *njs;

        struct {
            NativeAssets **list;
            uint32_t allocated;
            uint32_t size;
        } assetsList;
};

#endif

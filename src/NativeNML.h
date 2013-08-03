#ifndef nativenml_h__
#define nativenml_h__

#include "NativeStream.h"
#include <native_netlib.h>
#include "NativeAssets.h"
#include "external/rapidxml.hpp"
#include "NativeTypes.h"

class NativeNML;
class NativeJS;

typedef void (NativeNML::*tag_callback)(rapidxml::xml_node<> &node);

class NativeNML : public NativeStreamDelegate
{
    public:

    NativeNML(ape_global *net);
    ~NativeNML();
    void loadFile(const char *filename);
    void loadData(char *data, size_t len);
    void loadAssets(rapidxml::xml_node<> &node);
    void onAssetsItemReady(NativeAssets::Item *item);
    void onAssetsBlockReady(NativeAssets *asset);

    
    void onGetContent(const char *data, size_t len);
    void onAvailableData(size_t len){};
    /*
    void onNFIOOpen(NativeFileIO *);
    void onNFIOError(NativeFileIO *, int errno){};
    void onNFIORead(NativeFileIO *, unsigned char *data, size_t len);
    void onNFIOWrite(NativeFileIO *, size_t written){};*/

    void setNJS(NativeJS *js) {
        this->njs = js;
    }

    private:

    void addAsset(NativeAssets *);
    ape_global *net;
    NativeStream *stream;
    char *relativePath;

    /* Define callbacks for tags in <application> */
    struct _nml_tags {
        const char *str;
        tag_callback cb; // Call : (this->*cb)()
        bool unique;
    } nml_tags[2] = {
        {"assets",   &NativeNML::loadAssets, false},
        {NULL,       NULL, false}
    };

    uint32_t nassets;

    NativeJS *njs;

    struct {
        NativeAssets **list;
        uint32_t allocated;
        uint32_t size;
    } assetsList;
};

#endif

#ifndef nativenml_h__
#define nativenml_h__

#include "NativeStream.h"
#include <native_netlib.h>
#include "NativeAssets.h"
#include "external/rapidxml.hpp"
#include "NativeTypes.h"

class NativeNML;
class NativeJS;

#define XML_VP_MAX_WIDTH 8000
#define XML_VP_MAX_HEIGHT 8000

#define XML_VP_DEFAULT_WIDTH 980
#define XML_VP_DEFAULT_HEIGHT 700

typedef void (*NMLLoadedCallback)(void *arg);

class NativeNML : public NativeStreamDelegate
{
    public:

    typedef enum {
        NIDIUM_XML_OK,
        NIDIUM_XML_ERR_VIEWPORT_SIZE
    } nidium_xml_ret_t;

    typedef nidium_xml_ret_t (NativeNML::*tag_callback)(rapidxml::xml_node<> &node);

    NativeNML(ape_global *net);
    ~NativeNML();
    void loadFile(const char *filename, NMLLoadedCallback cb, void *arg);

    nidium_xml_ret_t loadAssets(rapidxml::xml_node<> &node);
    nidium_xml_ret_t loadMeta(rapidxml::xml_node<> &node);

    void onAssetsItemReady(NativeAssets::Item *item);
    void onAssetsBlockReady(NativeAssets *asset);
    void onGetContent(const char *data, size_t len);
    void onAvailableData(size_t len){};

    const char *getMetaTitle() const {
        return this->meta.title;
    }
    int getMetaWidth() const {
        return this->meta.size.width;
    }
    int getMetaHeight() const {
        return this->meta.size.height;
    }
    const char *getPath() const {
        return this->relativePath;
    }
    /*
    void onNFIOOpen(NativeFileIO *);
    void onNFIOError(NativeFileIO *, int errno){};
    void onNFIORead(NativeFileIO *, unsigned char *data, size_t len);
    void onNFIOWrite(NativeFileIO *, size_t written){};*/

    void setNJS(NativeJS *js) {
        this->njs = js;
    }

    private:
    bool loadData(char *data, size_t len);
    void addAsset(NativeAssets *);
    ape_global *net;
    NativeStream *stream;
    char *relativePath;

    /* Define callbacks for tags in <application> */
    struct _nml_tags {
        const char *str;
        tag_callback cb; // Call : (this->*cb)()
        bool unique;
    } nml_tags[3] = {
        {"assets",   &NativeNML::loadAssets, false},
        {"meta", &NativeNML::loadMeta, true},
        {NULL,       NULL, false}
    };

    uint32_t nassets;

    NativeJS *njs;

    struct {
        char *title;
        struct {
            int width;
            int height;
        } size;
    } meta;

    struct {
        NativeAssets **list;
        uint32_t allocated;
        uint32_t size;
    } assetsList;
    NMLLoadedCallback loaded;
    void *loaded_arg;
};


#endif

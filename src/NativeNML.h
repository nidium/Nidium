#ifndef nativenml_h__
#define nativenml_h__


#include <native_netlib.h>
#include "NativeAssets.h"
#include "external/rapidxml.hpp"
#include "NativeTypes.h"
#include <NativeMessages.h>

#define XML_VP_MAX_WIDTH 8000
#define XML_VP_MAX_HEIGHT 8000
#define XML_VP_DEFAULT_WIDTH 980
#define XML_VP_DEFAULT_HEIGHT 700


class NativeNML;
class NativeJS;
class JSObject;
class NativeBaseStream;

typedef void (*NMLLoadedCallback)(void *arg);

class NativeNML : public NativeMessages
{
public:
    explicit NativeNML(ape_global *net);
    ~NativeNML();

    typedef enum {
        NIDIUM_XML_OK,
        NIDIUM_XML_ERR_VIEWPORT_SIZE,
        NIDIUM_XML_ERR_IDENTIFIER_TOOLONG,
        NIDIUM_XML_ERR_META_MISSING
    } nidium_xml_ret_t;

    typedef nidium_xml_ret_t (NativeNML::*tag_callback)(rapidxml::xml_node<> &node);

    void onMessage(const NativeSharedMessages::Message &msg);
    void loadFile(const char *filename, NMLLoadedCallback cb, void *arg);

    void loadDefaultItems(NativeAssets *assets);
    nidium_xml_ret_t loadAssets(rapidxml::xml_node<> &node);
    nidium_xml_ret_t loadMeta(rapidxml::xml_node<> &node);
    nidium_xml_ret_t loadLayout(rapidxml::xml_node<> &node);

    void onAssetsItemReady(NativeAssets::Item *item);
    void onAssetsBlockReady(NativeAssets *asset);
    void onGetContent(const char *data, size_t len);

    const char *getMetaTitle() const {
        return this->meta.title;
    }
    const char *getIdentifier() const {
        return this->meta.identifier;
    }
    int getMetaWidth() const {
        return this->meta.size.width;
    }
    int getMetaHeight() const {
        return this->meta.size.height;
    }
    
    rapidxml::xml_node<> *getLayout() const {
        return m_Layout;
    }

    JSObject *getJSObjectLayout() const {
        return m_JSObjectLayout;
    }

    JSObject *buildLayoutTree(rapidxml::xml_node<> &node);

    void setNJS(NativeJS *js);

    /*
        str must be null-terminated.
        str is going to be modified
    */
    static JSObject *BuildLST(JSContext *cx, char *str);

private:

    static JSObject *BuildLSTFromNode(JSContext *cx, rapidxml::xml_node<> &node);

    bool loadData(char *data, size_t len, rapidxml::xml_document<> &doc);
    void addAsset(NativeAssets *);
    ape_global *net;
    NativeBaseStream *stream;

    /* Define callbacks for tags in <application> */
    struct _nml_tags {
        const char *str;
        tag_callback cb; // Call : (this->*cb)()
        bool unique;
    } nml_tags[4] = {
        {"assets",   &NativeNML::loadAssets, false},
        {"meta", &NativeNML::loadMeta, true},
        {"layout", &NativeNML::loadLayout, true},
        {NULL,       NULL, false}
    };

    uint32_t nassets;

    NativeJS *njs;

    struct {
        char *identifier;
        char *title;
        bool loaded;
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

    rapidxml::xml_node<> *m_Layout;
    JSObject *m_JSObjectLayout;

    bool m_defaultItemsLoaded;
};


#endif

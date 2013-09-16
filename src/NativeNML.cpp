#include "NativeNML.h"
#include "NativeJS.h"

#include <string.h>
#include "NativeHash.h"
#include "NativeStream.h"
#include "NativeJS.h"
#include "NativeJSWindow.h"
#include "NativeJSDocument.h"
#include <jsapi.h>

NativeNML::NativeNML(ape_global *net) :
    net(net), stream(NULL), relativePath(NULL), nassets(0), njs(NULL)
{
    assetsList.size = 0;
    assetsList.allocated = 4;

    assetsList.list = (NativeAssets **)malloc(sizeof(NativeAssets *) * assetsList.allocated);

    this->meta.title = NULL;
    this->meta.size.width = 0;
    this->meta.size.height = 0;
}

NativeNML::~NativeNML()
{
    if (relativePath) {
        if (this->njs) {
            this->njs->setPath(NULL);
        }
        free(relativePath);
    }
    if (stream) {
        delete stream;
    }
    for (int i = 0; i < assetsList.size; i++) {
        delete assetsList.list[i];
    }
    free(assetsList.list);
    if (this->meta.title) {
        free(this->meta.title);
    }
}

void NativeNML::setNJS(NativeJS *js)
{
    this->njs = js;
    this->njs->setPath(this->relativePath);
}

void NativeNML::loadFile(const char *file, NMLLoadedCallback cb, void *arg)
{
    this->loaded = cb;
    this->loaded_arg = arg;

    this->relativePath = NativeStream::resolvePath(file, NativeStream::STREAM_RESOLVE_PATH);

    stream = new NativeStream(this->net, file);
    stream->setDelegate(this);
    stream->getContent();
}

void NativeNML::onAssetsItemReady(NativeAssets::Item *item)
{
    NMLTag tag;
    memset(&tag, 0, sizeof(NMLTag));
    size_t len = 0;
    const unsigned char *data = item->get(&len);

    tag.tag = item->getTagName();
    tag.id = item->getName();
    tag.content.data = data;
    tag.content.len = len;
    tag.content.isBinary = false;

    if (njs == NULL) {

    }

    switch(item->fileType) {
        case NativeAssets::Item::ITEM_SCRIPT:
        {
            size_t len = 0;
            const unsigned char *data = item->get(&len);
            njs->LoadScriptContent((const char *)data, len, item->getName());

            break;
        }
        case NativeAssets::Item::ITEM_NSS:
        {
            NativeJSdocument *jdoc = NativeJSdocument::getNativeClass(njs->cx);
            if (jdoc == NULL) {
                return;
            }
            jdoc->populateStyle(njs->cx, (const char *)data,
                len, item->getName());
            break;
        }
        default:
            break;
    }
    /* TODO: allow the callback to change content ? */

    NativeJSwindow::getNativeClass(njs)->assetReady(tag);
}

static void NativeNML_onAssetsItemRead(NativeAssets::Item *item, void *arg)
{
    class NativeNML *nml = (class NativeNML *)arg;

    nml->onAssetsItemReady(item);
}

void NativeNML::onAssetsBlockReady(NativeAssets *asset)
{
    this->nassets--;

    if (this->nassets == 0) {
        NativeJSwindow::getNativeClass(njs)->onReady();
    }
}

static void NativeNML_onAssetsReady(NativeAssets *assets, void *arg)
{
    class NativeNML *nml = (class NativeNML *)arg;

    nml->onAssetsBlockReady(assets);
}

void NativeNML::addAsset(NativeAssets *asset)
{
    this->nassets++;
    if (assetsList.size == assetsList.allocated) {
        assetsList.allocated *= 2;
        assetsList.list = (NativeAssets **)realloc(assetsList.list,
            sizeof(NativeAssets *) * assetsList.allocated);
    }

    assetsList.list[assetsList.size] = asset;

    assetsList.size++;
}

NativeNML::nidium_xml_ret_t NativeNML::loadMeta(rapidxml::xml_node<> &node)
{
    using namespace rapidxml;
    for (xml_node<> *child = node.first_node(); child != NULL;
        child = child->next_sibling())
    {
        if (strncasecmp(child->name(), "title", 5) == 0) {
            if (this->meta.title)
                free(this->meta.title);

            this->meta.title = (char *)malloc(sizeof(char) *
                (child->value_size() + 1));

            memcpy(this->meta.title, child->value(), child->value_size());
            this->meta.title[child->value_size()] = '\0';

        } else if (strncasecmp(child->name(), "viewport", 8) == 0) {
            char *pos;
            if ((pos = (char *)memchr(child->value(), 'x',
                child->value_size())) == NULL) {

                return NIDIUM_XML_ERR_VIEWPORT_SIZE;
            }
            *pos = '\0';
            int width = atoi(child->value());
            if (width < 1 || width > XML_VP_MAX_WIDTH) {
                return NIDIUM_XML_ERR_VIEWPORT_SIZE;
            }
            this->meta.size.width = width;
            *(char *)(child->value()+child->value_size()) = '\0';

            int height = atoi(pos+1);

            if (height < 0 || height > XML_VP_MAX_HEIGHT) {
                return NIDIUM_XML_ERR_VIEWPORT_SIZE;
            }
            this->meta.size.height = height;
        }
    }

    if (this->getMetaWidth() == 0) {
        this->meta.size.width = XML_VP_DEFAULT_WIDTH;
    }
    if (this->getMetaHeight() == 0) {
        this->meta.size.height = XML_VP_DEFAULT_HEIGHT;
    }

    return NIDIUM_XML_OK;
}

NativeNML::nidium_xml_ret_t NativeNML::loadAssets(rapidxml::xml_node<> &node)
{
    using namespace rapidxml;

    NativeAssets *assets = new NativeAssets(NativeNML_onAssetsItemRead,
        NativeNML_onAssetsReady, this);

    this->addAsset(assets);

    for (xml_node<> *child = node.first_node(); child != NULL;
        child = child->next_sibling())
    {
        xml_attribute<> *src = NULL;
        NativeAssets::Item *item = NULL;

        if ((src = child->first_attribute("src"))) {
            xml_attribute<> *id = child->first_attribute("id");
            item = new NativeAssets::Item(src->value(),
                NativeAssets::Item::ITEM_UNKNOWN, net, this->relativePath);

            /* Name could be automatically changed afterward */
            item->setName(src->value());

            assets->addToPendingList(item);
        } else {
            item = new NativeAssets::Item(NULL, NativeAssets::Item::ITEM_UNKNOWN, net);
            item->setName("inline"); /* TODO: NML name */
            assets->addToPendingList(item);
            item->setContent(child->value(), child->value_size(), true);
        }

        item->setTagName(child->name());
        
        if (!strncasecmp(child->name(), "script", 6)) {
            item->fileType = NativeAssets::Item::ITEM_SCRIPT;
        } else if (!strncasecmp(child->name(), "style", 6)) {
            item->fileType = NativeAssets::Item::ITEM_NSS;
        }
        //printf("Node : %s\n", child->name());
    }

    assets->endListUpdate(net);

    return NIDIUM_XML_OK;
}


bool NativeNML::loadData(char *data, size_t len)
{
    using namespace rapidxml;

    xml_document<> doc;

    try {
        doc.parse<0>(data);
    } catch(rapidxml::parse_error &err) {
        printf("XML error : %s\n", err.what());

        return false;
    }

    xml_node<> *node = doc.first_node("application");
    if (node == NULL) {
        printf("XML : <application> node not found\n");
        return false;
    }

    for (xml_node<> *child = node->first_node(); child != NULL;
        child = child->next_sibling())
    {
        for (int i = 0; nml_tags[i].str != NULL; i++) {
            if (!strncasecmp(nml_tags[i].str, child->name(),
                child->name_size())) {

                nidium_xml_ret_t ret;

                if ((ret = (this->*nml_tags[i].cb)(*child)) != NIDIUM_XML_OK) {
                    printf("XML : Nidium error (%d)\n", ret);
                    return false;
                }
            }
        }
    }

    return true;
}

void NativeNML::onGetContent(const char *data, size_t len)
{
    if (this->loadData((char *)data, len)) {
        this->loaded(this->loaded_arg);
    }
}

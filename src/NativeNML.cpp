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

void NativeNML::loadFile(const char *file, NMLLoadedCallback cb, void *arg)
{
    this->loaded = cb;
    this->loaded_arg = arg;

    this->relativePath = NativeStream::resolvePath(file, NativeStream::STREAM_RESOLVE_PATH);

    if (this->njs) {
        this->njs->setPath(this->relativePath);
    }

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
        assetsList.list = (NativeAssets **)realloc(assetsList.list, sizeof(NativeAssets *) * assetsList.allocated);
    }

    assetsList.list[assetsList.size] = asset;

    assetsList.size++;
}

void NativeNML::loadMeta(rapidxml::xml_node<> &node)
{
    using namespace rapidxml;
    for (xml_node<> *child = node.first_node(); child != NULL;
        child = child->next_sibling())
    {
        if (strncasecmp(child->name(), "title", 5) == 0) {
            if (this->meta.title) free(this->meta.title);
            this->meta.title = (char *)malloc(sizeof(char) * (child->value_size() + 1));
            memcpy(this->meta.title, child->value(), child->value_size());
            this->meta.title[child->value_size()] = '\0';
        }
        printf("Parsed : %s\n", child->name());
    }    
}

void NativeNML::loadAssets(rapidxml::xml_node<> &node)
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

                (this->*nml_tags[i].cb)(*child);
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

/*
   Copyright 2016 Nidium Inc. All rights reserved.
   Use of this source code is governed by a MIT license
   that can be found in the LICENSE file.
*/
#include "Frontend/NML.h"

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <strings.h>
#include <unistd.h>

#include "Binding/JSUtils.h"

#include "Interface/SystemInterface.h"
#include "Macros.h"

#include "Binding/JSWindow.h"
#include "Binding/JSDocument.h"
#include "Binding/ThreadLocalContext.h"

using Nidium::Core::SharedMessages;
using Nidium::Core::Path;
using Nidium::IO::Stream;
using Nidium::Binding::NidiumJS;
using Nidium::Binding::JSUtils;
using Nidium::Binding::JSWindow;
using Nidium::Binding::JSDocument;
using Nidium::Binding::ClassMapper;
using Nidium::Binding::JSObjectBuilder;
using Nidium::Interface::SystemInterface;

namespace Nidium {
namespace Frontend {

// {{{ NML
/*@FIXME:: refractor the constructor, so that m_JSObjectLayout get's
 * njs'javascript context*/
NML::NML(ape_global *net)
    : m_Net(net), m_Stream(NULL), m_nAssets(0), m_Njs(NULL), m_Loaded(NULL),
      m_LoadedArg(NULL), m_Layout(NULL), m_JSObjectLayout(NULL),
      m_DefaultItemsLoaded(false), m_LoadFramework(true), m_LoadHTML5(false)
{
    m_AssetsList.size      = 0;
    m_AssetsList.allocated = 4;

    m_AssetsList.list = static_cast<Assets **>(
        malloc(sizeof(Assets *) * m_AssetsList.allocated));

    m_Meta.title       = NULL;
    m_Meta.size.width  = 0;
    m_Meta.size.height = 0;
    m_Meta.identifier  = NULL;

    memset(&m_Meta, 0, sizeof(m_Meta));
}

void NML::setNJS(NidiumJS *js)
{
    m_Njs = js;
    /*
    if (m_JSObjectLayout.get()) {
        @FIXME: actually check that it is a context: even better: set the
    context here, and not in
    the constructor
        m_JSObjectLayout.remove();
        m_JSObjectLayout(js->getJSContext());
    }
    */
}

void NML::loadFile(const char *file, NMLLoadedCallback cb, void *arg)
{
    m_Loaded    = cb;
    m_LoadedArg = arg;

    Path path(file);

    nlogf("NML path : %s", path.path());

    m_Stream = Stream::Create(path);
    if (m_Stream == NULL) {
        SystemInterface::GetInstance()->alert("NML error : stream error",
                                              SystemInterface::ALERT_CRITIC);
        exit(1);
    }
    /*
        Set the global working directory at the NML location
    */
    Path::CD(path.dir());
    Path::Chroot(path.dir());

    m_Stream->setListener(this);
    m_Stream->getContent();
}

void NML::loadDefaultItems(Assets *assets)
{
    if (m_DefaultItemsLoaded) {
        return;
    }

    m_DefaultItemsLoaded = true;

    Assets::Item *preload = new Assets::Item("embed://preload.js",
                                             Assets::Item::ITEM_SCRIPT, m_Net);
    preload->setTagName("__NidiumPreload__");

    assets->addToPendingList(preload);
}

bool NML::loadData(char *data, size_t len, rapidxml::xml_document<> &doc)
{
    using namespace rapidxml;

    if (len == 0 || data == NULL) {
        SystemInterface::GetInstance()->alert("NML error : empty file",
                                              SystemInterface::ALERT_CRITIC);
        return false;
    }

    try {
        doc.parse<0>(data);
    } catch (rapidxml::parse_error &err) {
        char cerr[2048];

        sprintf(cerr, "NML error : %s", err.what());
        SystemInterface::GetInstance()->alert(cerr,
                                              SystemInterface::ALERT_CRITIC);

        return false;
    }

    xml_node<> *node = doc.first_node("application");
    if (node == NULL) {
        SystemInterface::GetInstance()->alert("<application> node not found",
                                              SystemInterface::ALERT_CRITIC);
        return false;
    }

    xml_attribute<char> *framework
        = node->first_attribute(CONST_STR_LEN("framework"), false);
    xml_attribute<char> *html5
        = node->first_attribute(CONST_STR_LEN("html5"), false);

    if (framework) {
        if (strncasecmp(framework->value(), CONST_STR_LEN("false")) == 0) {
            m_LoadFramework = false;
        }
    }

    if (html5) {
        if (strncasecmp(html5->value(), CONST_STR_LEN("true")) == 0) {
            m_LoadHTML5 = true;
        }
    }

    for (xml_node<> *child = node->first_node(); child != NULL;
         child = child->next_sibling()) {
        for (int i = 0; m_NmlTags[i].str != NULL; i++) {
            if (!strncasecmp(m_NmlTags[i].str, child->name(),
                             child->name_size())) {

                nidium_xml_ret_t ret;

                if ((ret = (this->*m_NmlTags[i].cb)(*child)) != NIDIUM_XML_OK) {
                    ndm_logf(NDM_LOG_ERROR, "NML", "XML : Nidium error (%d)", ret);
                    SystemInterface::GetInstance()->alert(
                        "NML ERROR", SystemInterface::ALERT_CRITIC);
                    return false;
                }
            }
        }
    }

    if (!m_Meta.loaded) {
        SystemInterface::GetInstance()->alert(
            "<meta> tag is missing in the NML file",
            SystemInterface::ALERT_CRITIC);
        return false;
    }

    return true;
}


JSObject *NML::BuildLST(JSContext *cx, char *str)
{
    using namespace rapidxml;

    rapidxml::xml_document<> doc;

    try {
        doc.parse<0>(str);
    } catch (rapidxml::parse_error &err) {
        return NULL;
    }

    return BuildLSTFromNode(cx, doc);
}

JSObject *NML::BuildLSTFromNode(JSContext *cx, rapidxml::xml_node<> &node)
{
#define NODE_PROP(where, name, val)         \
    JS_DefineProperty(cx, where, name, val, \
                      JSPROP_PERMANENT | JSPROP_READONLY | JSPROP_ENUMERATE)
#define NODE_STR(data, len)                         \
    JSUtils::NewStringWithEncoding( \
        cx, static_cast<const char *>(data), len, "utf8")

    using namespace rapidxml;

    JS::RootedObject input(cx, JS_NewArrayObject(cx, 0));

    uint32_t idx = 0;
    for (xml_node<> *child = node.first_node(); child != NULL;
         child = child->next_sibling()) {
        JS::RootedObject obj(cx, JS_NewPlainObject(cx));
        bool skip = false;

        switch (child->type()) {
            case node_data:
            case node_cdata: {
                JS::RootedString typeStr(cx, NODE_STR("textNode", 8));
                JS::RootedString childStr(
                    cx, NODE_STR(child->value(), child->value_size()));
                NODE_PROP(obj, "type", typeStr);
                NODE_PROP(obj, "text", childStr);
            } break;
            case node_element: {
                JS::RootedString typeStr(
                    cx, NODE_STR(child->name(), child->name_size()));
                NODE_PROP(obj, "type", typeStr);
                JS::RootedObject obj_attr(
                    cx, JS_NewPlainObject(cx));
                NODE_PROP(obj, "attributes", obj_attr);
                for (xml_attribute<> *attr = child->first_attribute();
                     attr != NULL; attr = attr->next_attribute()) {
                    JS::RootedString str(
                        cx, NODE_STR(attr->value(), attr->value_size()));
                    NODE_PROP(obj_attr, attr->name(), str);
                }
                JS::RootedObject obj_children(cx, BuildLSTFromNode(cx, *child));
                NODE_PROP(obj, "children", obj_children);
            } break;
            default:
                skip = true;
                break;
        }

        if (skip) {
            continue;
        }
        /* push to input array */
        JS_SetElement(cx, input, idx++, obj);
    }
    return input;
#undef NODE_PROP
}

/*
    <canvas>
        <next></next>
    </canvas>
    <foo></foo>
*/
JSObject *NML::buildLayoutTree(rapidxml::xml_node<> &node)
{
    return BuildLSTFromNode(m_Njs->m_Cx, node);
}

static int delete_stream(void *arg)
{
    Stream *stream = static_cast<Stream *>(arg);

    delete stream;

    return 0;
}

void NML::onMessage(const SharedMessages::Message &msg)
{
    switch (msg.event()) {
        case Stream::kEvents_ReadBuffer: {
            buffer *buf = static_cast<buffer *>(msg.m_Args[0].toPtr());

            /*
                Some stream can have dynamic path (e.g http 301 or 302).
                We make sure to update the root path in that case
            */
            const char *streamPath = m_Stream->getPath();

            if (streamPath != NULL) {
                Path path(streamPath);
                Path::CD(path.dir());
                Path::Chroot(path.dir());
            }

            this->onGetContent((const char *)buf->data, buf->used);
            break;
        }
        case Stream::kEvents_Error: {
            SystemInterface::GetInstance()->alert(
                "NML error : stream error", SystemInterface::ALERT_CRITIC);
            exit(1);
            break;
        }
        default:
            break;
    }
}

void NML::onGetContent(const char *data, size_t len)
{
    rapidxml::xml_document<> doc;

    char *data_nullterminated;
    bool needRelease = false;

    if (data[len] != '\0') {
        data_nullterminated = static_cast<char *>(malloc(len + 1));
        memcpy(data_nullterminated, data, len);
        data_nullterminated[len] = '\0';
        needRelease              = true;
    } else {
        // TODO: new style cast
        data_nullterminated = (char *)(data);
    }

    if (this->loadData(data_nullterminated, len, doc)) {

        if (m_Layout) {
            m_JSObjectLayout = this->buildLayoutTree(*m_Layout);
            Binding::NidiumLocalContext::RootObjectUntilShutdown(m_JSObjectLayout);
        }
    } else {
        /*
            TODO: Do not close ! (load a default NML?)
        */
        exit(1);
    }

    /* Invalidate layout node since memory pool is free'd */
    m_Layout = NULL;
    /* Stream has ended */
    ape_global *ape = m_Net;
    timer_dispatch_async(delete_stream, m_Stream);
    m_Stream = NULL;

    if (needRelease) {
        free(data_nullterminated);
    }
}

NML::~NML()
{
    if (m_JSObjectLayout.get()) {
        Binding::NidiumLocalContext::UnrootObject(m_JSObjectLayout);
        m_JSObjectLayout = nullptr;
    }

    if (m_Stream) {
        delete m_Stream;
    }
    for (int i = 0; i < m_AssetsList.size; i++) {
        delete m_AssetsList.list[i];
    }
    free(m_AssetsList.list);
    if (m_Meta.title) {
        free(m_Meta.title);
    }

    Path::CD(NULL);
    Path::Chroot(NULL);
}
// }}}

// {{{ Assets
void NML::onAssetsItemReady(Assets::Item *item)
{
    NMLTag tag;
    memset(&tag, 0, sizeof(NMLTag));
    size_t len = 0;

    const unsigned char *data = item->get(&len);

    tag.tag              = item->getTagName();
    tag.id               = item->getName();
    tag.content.data     = data;
    tag.content.len      = len;
    tag.content.isBinary = false;

    if (data != NULL) {

        switch (item->m_FileType) {
            case Assets::Item::ITEM_SCRIPT: {
                m_Njs->LoadScriptContent((char *)data, len, item->getName());

                if (strcmp(item->getTagName(), "__NidiumPreload__") == 0) {
                    JSContext *cx = m_Njs->getJSContext();
                    JS::RootedObject gbl(cx, JS::CurrentGlobalOrNull(cx));
                    JS::AutoValueArray<2> args(cx);
                    JSObjectBuilder obj(cx);
                    JS::RootedValue rval(cx);

                    args[0].setObjectOrNull(obj.obj());

                    obj.set("framework", m_LoadFramework);
                    obj.set("html5", m_LoadHTML5);

                    args[1].setObjectOrNull(m_JSObjectLayout);

                    JS_CallFunctionName(cx, gbl, "__nidiumPreload", args,
                                        &rval);

                    if (JS_IsExceptionPending(cx)) {
                        if (!JS_ReportPendingException(cx)) {
                            JS_ClearPendingException(cx);
                        }
                    }
                }

                break;
            }
            case Assets::Item::ITEM_NSS: {
                JSDocument *jdoc = JSDocument::GetInstanceSingleton();
                if (jdoc == NULL) {
                    return;
                }
                // TODO: new style cast
                jdoc->populateStyle(m_Njs->m_Cx, (const char *)(data), len,
                                    item->getName());
                break;
            }
            default:
                break;
        }
    }
    /* TODO: allow the callback to change content ? */

    JSWindow::GetObject(m_Njs)->assetReady(tag);
}

static void NML_onAssetsItemRead(Assets::Item *item, void *arg)
{
    class NML *nml = static_cast<class NML *>(arg);

    nml->onAssetsItemReady(item);
}

void NML::onAssetsBlockReady(Assets *asset)
{
    m_nAssets--;

    if (m_nAssets == 0) {
        JS::RootedObject layoutObj(m_Njs->m_Cx, m_JSObjectLayout);
        JSWindow::GetObject(m_Njs)->onReady(layoutObj);
    }
}

static void NML_onAssetsReady(Assets *assets, void *arg)
{
    class NML *nml = static_cast<class NML *>(arg);

    nml->onAssetsBlockReady(assets);
}

void NML::addAsset(Assets *asset)
{
    m_nAssets++;
    if (m_AssetsList.size == m_AssetsList.allocated) {
        m_AssetsList.allocated *= 2;
        m_AssetsList.list = static_cast<Assets **>(realloc(
            m_AssetsList.list, sizeof(Assets *) * m_AssetsList.allocated));
    }

    m_AssetsList.list[m_AssetsList.size] = asset;

    m_AssetsList.size++;
}
// }}}

// {{{ xml
NML::nidium_xml_ret_t NML::loadMeta(rapidxml::xml_node<> &node)
{
    using namespace rapidxml;

    if (m_Meta.loaded) {
        return NIDIUM_XML_ERR_META_MULTIPLE;
    }

    for (xml_node<> *child = node.first_node(); child != NULL;
         child = child->next_sibling()) {
        if (strncasecmp(child->name(), "title", 5) == 0) {
            if (m_Meta.title) free(m_Meta.title);

            m_Meta.title = static_cast<char *>(
                malloc(sizeof(char) * (child->value_size() + 1)));

            memcpy(m_Meta.title, child->value(), child->value_size());
            m_Meta.title[child->value_size()] = '\0';

        } else if (strncasecmp(child->name(), "viewport", 8) == 0) {
            char *pos;
            if ((pos = static_cast<char *>(
                     memchr(child->value(), 'x', child->value_size())))
                == NULL) {

                return NIDIUM_XML_ERR_VIEWPORT_SIZE;
            }
            *pos      = '\0';
            int width = atoi(child->value());
            if (width < 1 || width > XML_VP_MAX_WIDTH) {
                return NIDIUM_XML_ERR_VIEWPORT_SIZE;
            }
            m_Meta.size.width = width;
            *static_cast<char *>(child->value() + child->value_size()) = '\0';

            int height = atoi(pos + 1);

            if (height < 0 || height > XML_VP_MAX_HEIGHT) {
                return NIDIUM_XML_ERR_VIEWPORT_SIZE;
            }
            m_Meta.size.height = height;
        } else if (strncasecmp(child->name(), "identifier", 10) == 0) {
            if (child->value_size() > 128) {
                return NIDIUM_XML_ERR_IDENTIFIER_TOOLONG;
            }
            if (m_Meta.identifier) free(m_Meta.identifier);

            m_Meta.identifier = static_cast<char *>(
                malloc(sizeof(char) * (child->value_size() + 1)));

            memcpy(m_Meta.identifier, child->value(), child->value_size());
            m_Meta.identifier[child->value_size()] = '\0';
        }
    }

    if (this->getMetaWidth() == 0) {
        m_Meta.size.width = XML_VP_DEFAULT_WIDTH;
    }
    if (this->getMetaHeight() == 0) {
        m_Meta.size.height = XML_VP_DEFAULT_HEIGHT;
    }

    m_Meta.loaded = true;

    m_Loaded(m_LoadedArg);

    return NIDIUM_XML_OK;
}

NML::nidium_xml_ret_t NML::loadAssets(rapidxml::xml_node<> &node)
{

    if (!m_Meta.loaded) return NIDIUM_XML_ERR_META_MISSING;

    using namespace rapidxml;

    Assets *assets = new Assets(NML_onAssetsItemRead, NML_onAssetsReady, this);

    this->addAsset(assets);
    this->loadDefaultItems(assets);

    for (xml_node<> *child = node.first_node(); child != NULL;
         child               = child->next_sibling()) {
        xml_attribute<> *src = NULL;
        Assets::Item *item   = NULL;

        if ((src = child->first_attribute("src"))) {
            item = new Assets::Item(src->value(), Assets::Item::ITEM_UNKNOWN,
                                    m_Net);

            /* Name could be automatically changed afterward */
            item->setName(src->value());

            assets->addToPendingList(item);
        } else {
            item = new Assets::Item(NULL, Assets::Item::ITEM_UNKNOWN, m_Net);
            item->setName("inline"); /* TODO: NML name */
            assets->addToPendingList(item);
            item->setContent(child->value(), child->value_size(), true);
        }

        item->setTagName(child->name());

        if (!strncasecmp(child->name(), CONST_STR_LEN("script"))) {
            item->m_FileType = Assets::Item::ITEM_SCRIPT;
        } else if (!strncasecmp(child->name(), CONST_STR_LEN("style"))) {
            item->m_FileType = Assets::Item::ITEM_NSS;
        }
        // ndm_logf(NDM_LOG_DEBUG, "NML", "Node : %s", child->name());
    }

    assets->endListUpdate(m_Net);

    return NIDIUM_XML_OK;
}

NML::nidium_xml_ret_t NML::loadLayout(rapidxml::xml_node<> &node)
{
    if (!m_Meta.loaded) return NIDIUM_XML_ERR_META_MISSING;

    m_Layout = node.document()->clone_node(&node);

    return NIDIUM_XML_OK;
}

// }}}

} // namespace Frontend
} // namespace Nidium

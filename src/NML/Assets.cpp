#include "NML/Assets.h"

#include <stdlib.h>
#include <stdbool.h>
#include <string.h>

NativeAssets::NativeAssets(readyItem cb, readyAssets rcb, void *arg) :
    m_ItemReady(cb), m_AssetsReady(rcb), m_ReadyArg(arg), m_Nitems(0)
{
    m_Pending_list.head = NULL;
    m_Pending_list.foot = NULL;
}

NativeAssets::~NativeAssets()
{
    struct item_list *il = m_Pending_list.head, *ilnext;
    while (il != NULL) {
        ilnext = il->next;
        delete il->item;
        free(il);
        il = ilnext;
    }
}

NativeAssets::Item::Item(const char *url, FileType t,
    ape_global *net) :
    m_FileType(t), m_State(ITEM_LOADING), m_Stream(NULL),
    m_Url(url), m_Net(net), m_Assets(NULL), m_Name(NULL), m_Tagname(NULL)
{
    m_Data.data = NULL;
    m_Data.len = 0;
}


void NativeAssets::Item::onMessage(const Nidium::Core::SharedMessages::Message &msg)
{
    switch (msg.event()) {
        case Nidium::IO::Stream::EVENT_READ_BUFFER:
        {
            buffer *buf = (buffer *)msg.args[0].toPtr();
            this->setContent((const char *)buf->data, buf->used);
            break;
        }
        case Nidium::IO::Stream::EVENT_ERROR:
        {
            this->setContent(NULL, 0);
            break;
        }
        default:
            break;
    }
}

NativeAssets::Item::~Item()
{
    if (m_Name) {
        free(m_Name);
    }

    if (m_Tagname) {
        free(m_Tagname);
    }
    if (m_Data.data) {
        free(m_Data.data);
    }

    if (m_Stream) {
        delete m_Stream;
    }
}

void NativeAssets::Item::download()
{
    m_Stream = Nidium::IO::Stream::create(Nidium::Core::Path(m_Url));

    if (m_Stream == NULL) {
        this->setName(m_Url);

        /*
            setContent to NULL .
            In async way (3rd param) so that NativeContext
            has time to set setNJS().
        */
        this->setContent(NULL, 0, true);
        return;
    }

    /* Reset the name with the new location forged by NativeStream */
    this->setName(m_Stream->getLocation());

    m_Stream->setListener(this);
    m_Stream->getContent();
}

static int NativeAssets_pendingListUpdate(void *arg)
{
    NativeAssets *assets = static_cast<NativeAssets *>(arg);

    assets->pendingListUpdate();

    return 0;
}

void NativeAssets::Item::setContent(const char *data, size_t len, bool async) {
    m_State = ITEM_LOADED;

    if (len) {
        m_Data.data = (unsigned char *)malloc(len);
        memcpy(m_Data.data, data, len);
    } else {
        m_Data.data = NULL;
    }
    m_Data.len  = len;
    if (m_Assets) {
        if (async) {
            ape_global *ape = m_Net;
            timer_dispatch_async_unprotected(NativeAssets_pendingListUpdate, m_Assets);
        } else {
            m_Assets->pendingListUpdate();
        }
    }
}

void NativeAssets::addToPendingList(Item *item)
{
    struct item_list *il = (struct item_list *)malloc(sizeof(*il));

    m_Nitems++;
    il->item = item;
    il->next = NULL;
    item->m_State = NativeAssets::Item::ITEM_LOADING;
    item->m_Assets = this;

    if (m_Pending_list.head == NULL) {
        m_Pending_list.head = il;
    }

    if (m_Pending_list.foot != NULL) {
        m_Pending_list.foot->next = il;
    }

    m_Pending_list.foot = il;

    if (item->m_Url != NULL) {
        item->download();
    }
}

static int NativeAssets_deleteItem(void *arg)
{
    NativeAssets::Item *item = (NativeAssets::Item *)arg;

    delete item;

    return 0;
}

void NativeAssets::endListUpdate(ape_global *ape)
{
    if (m_Nitems == 0) {
        timer_dispatch_async_unprotected(NativeAssets_pendingListUpdate, this);
    }
}

void NativeAssets::pendingListUpdate()
{
    bool worked = false;
    struct item_list *il = m_Pending_list.head, *ilnext;
    while (il != NULL && il->item->m_State == NativeAssets::Item::ITEM_LOADED) {
        m_Nitems--;
        m_ItemReady(il->item, m_ReadyArg);

        m_Pending_list.head = il->next;

        if (il->next == NULL) {
            m_Pending_list.foot = NULL;
        }

        ilnext = il->next;
        ape_global *ape = il->item->m_Net;
        timer_dispatch_async_unprotected(NativeAssets_deleteItem, il->item);
        free(il);

        il = ilnext;

        worked = true;
    }

    if (m_Nitems == 0 && worked) {
        m_AssetsReady(this, m_ReadyArg);
    }
}


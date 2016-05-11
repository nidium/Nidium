/*
   Copyright 2016 Nidium Inc. All rights reserved.
   Use of this source code is governed by a MIT license
   that can be found in the LICENSE file.
*/
#include "Frontend/Assets.h"

#include <stdlib.h>
#include <stdbool.h>
#include <string.h>

using Nidium::Core::SharedMessages;
using Nidium::Core::Path;
using Nidium::IO::Stream;

namespace Nidium {
namespace Frontend {

Assets::Assets(readyItem cb, readyAssets rcb, void *arg) :
    m_ItemReady(cb), m_AssetsReady(rcb), m_ReadyArg(arg), m_Nitems(0)
{
    m_Pending_list.head = NULL;
    m_Pending_list.foot = NULL;
}

Assets::~Assets()
{
    struct item_list *il = m_Pending_list.head, *ilnext;
    while (il != NULL) {
        ilnext = il->next;
        delete il->item;
        free(il);
        il = ilnext;
    }
}

Assets::Item::Item(const char *url, FileType t,
    ape_global *net) :
    m_FileType(t), m_State(ITEM_LOADING), m_Stream(NULL),
    m_Url(url), m_Net(net), m_Assets(NULL), m_Name(NULL), m_Tagname(NULL)
{
    m_Data.data = NULL;
    m_Data.len = 0;
}

void Assets::Item::onMessage(const SharedMessages::Message &msg)
{
    switch (msg.event()) {
        case Stream::kEvents_ReadBuffer:
        {
            //TODO: new style cast
            buffer *buf = reinterpret_cast<buffer *>(msg.m_Args[0].toPtr());
            this->setContent((const char *)(buf->data), buf->used);
            break;
        }
        case Stream::kEvents_Error:
        {
            this->setContent(NULL, 0);
            break;
        }
        default:
            break;
    }
}

Assets::Item::~Item()
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

void Assets::Item::download()
{
    m_Stream = Stream::Create(Path(m_Url));

    if (m_Stream == NULL) {
        this->setName(m_Url);

        /*
            setContent to NULL .
            In async way (3rd param) so that Context
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

static int Assets_pendingListUpdate(void *arg)
{
    Assets *assets = static_cast<Assets *>(arg);

    assets->pendingListUpdate();

    return 0;
}

void Assets::Item::setContent(const char *data, size_t len, bool async) {
    m_State = ITEM_LOADED;

    if (len) {
        m_Data.data = static_cast<unsigned char *>(malloc(len));
        memcpy(m_Data.data, data, len);
    } else {
        m_Data.data = NULL;
    }
    m_Data.len  = len;
    if (m_Assets) {
        if (async) {
            ape_global *ape = m_Net;
            timer_dispatch_async_unprotected(Assets_pendingListUpdate, m_Assets);
        } else {
            m_Assets->pendingListUpdate();
        }
    }
}

void Assets::addToPendingList(Item *item)
{
    struct item_list *il = (struct item_list *)malloc(sizeof(*il));

    m_Nitems++;
    il->item = item;
    il->next = NULL;
    item->m_State = Assets::Item::ITEM_LOADING;
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

static int Assets_deleteItem(void *arg)
{
    Assets::Item *item = (Assets::Item *)arg;

    delete item;

    return 0;
}

void Assets::endListUpdate(ape_global *ape)
{
    if (m_Nitems == 0) {
        timer_dispatch_async_unprotected(Assets_pendingListUpdate, this);
    }
}

void Assets::pendingListUpdate()
{
    bool worked = false;
    struct item_list *il = m_Pending_list.head, *ilnext;
    while (il != NULL && il->item->m_State == Assets::Item::ITEM_LOADED) {
        m_Nitems--;
        m_ItemReady(il->item, m_ReadyArg);

        m_Pending_list.head = il->next;

        if (il->next == NULL) {
            m_Pending_list.foot = NULL;
        }

        ilnext = il->next;
        ape_global *ape = il->item->m_Net;
        timer_dispatch_async_unprotected(Assets_deleteItem, il->item);
        free(il);

        il = ilnext;

        worked = true;
    }

    if (m_Nitems == 0 && worked) {
        m_AssetsReady(this, m_ReadyArg);
    }
}

} // namespace Frontend
} // namespace Nidium


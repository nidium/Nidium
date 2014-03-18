#include "NativeAssets.h"
#include <native_netlib.h>
#include <NativePath.h>

NativeAssets::NativeAssets(readyItem cb, readyAssets rcb, void *arg) :
    itemReady(cb), assetsReady(rcb), readyArg(arg), nitems(0)
{
    pending_list.head = NULL;
    pending_list.foot = NULL;
}

NativeAssets::~NativeAssets()
{
    struct item_list *il = pending_list.head, *ilnext;
    while (il != NULL) {
        ilnext = il->next;
        delete il->item;
        free(il);
        il = ilnext;
    }
}

NativeAssets::Item::Item(const char *url, FileType t,
    ape_global *net) :
    fileType(t), state(ITEM_LOADING), stream(NULL),
    url(url), net(net), assets(NULL), name(NULL), tagname(NULL)
{
    data.data = NULL;
    data.len = 0;
}


void NativeAssets::Item::onMessage(const NativeSharedMessages::Message &msg)
{
    switch (msg.event()) {
        case NATIVESTREAM_READ_BUFFER:
        {
            buffer *buf = (buffer *)msg.args[0].toPtr();
            this->setContent((const char *)buf->data, buf->used);
            break;
        }
        case NATIVESTREAM_ERROR:
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
    if (name) {
        free(name);
    }

    if (tagname) {
        free(tagname);
    }
    if (this->data.data) {
        free(this->data.data);
    }

    if (stream) {
        delete stream;
    }
}

void NativeAssets::Item::download()
{
    this->stream = NativeBaseStream::create(NativePath(url));

    /* Reset the name with the new location forged by NativeStream */
    this->setName(this->stream->getLocation());

    stream->setListener(this);
    stream->getContent();
}

static int NativeAssets_pendingListUpdate(void *arg)
{
    NativeAssets *assets = (NativeAssets *)arg;

    assets->pendingListUpdate();

    return 0;
}

void NativeAssets::Item::setContent(const char *data, size_t len, bool async) {
    this->state = ITEM_LOADED;

    if (len) {
        this->data.data = (unsigned char *)malloc(len);
        memcpy(this->data.data, data, len);
    } else {
        this->data.data = NULL;
    }
    this->data.len  = len;
    if (assets) {
        if (async) {
            ape_global *ape = this->net;
            timer_dispatch_async_unprotected(NativeAssets_pendingListUpdate, assets);
        } else {
            assets->pendingListUpdate();
        }
    }
}

void NativeAssets::addToPendingList(Item *item)
{
    struct item_list *il = (struct item_list *)malloc(sizeof(*il));

    this->nitems++;

    il->item = item;
    il->next = NULL;
    item->state = NativeAssets::Item::ITEM_LOADING;
    item->assets = this;

    if (pending_list.head == NULL) {
        pending_list.head = il;
    }

    if (pending_list.foot != NULL) {
        pending_list.foot->next = il;
    }

    pending_list.foot = il;

    if (item->url != NULL) {
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
    if (this->nitems == 0) {
        timer_dispatch_async_unprotected(NativeAssets_pendingListUpdate, this);
    }
}

void NativeAssets::pendingListUpdate()
{
    struct item_list *il = pending_list.head, *ilnext;
    while (il != NULL && il->item->state == NativeAssets::Item::ITEM_LOADED) {
        this->nitems--;
        itemReady(il->item, readyArg);

        pending_list.head = il->next;

        if (il->next == NULL) {
            pending_list.foot = NULL;
        }

        ilnext = il->next;
        ape_global *ape = il->item->net;
        timer_dispatch_async_unprotected(NativeAssets_deleteItem, il->item);
        free(il);

        il = ilnext;
    }

    if (this->nitems == 0) {
        assetsReady(this, readyArg);
    }
}

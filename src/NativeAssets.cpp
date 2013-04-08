#include "NativeAssets.h"
#include <native_netlib.h>

NativeAssets::NativeAssets(readyItem cb, void *arg) :
    itemReady(cb), readyArg(arg)
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

NativeAssets::Item::Item(const char *url, FileType t, ape_global *net) :
    fileType(t), state(ITEM_LOADING), stream(NULL),
    url(url), net(net), assets(NULL), name(NULL), tagname(NULL)
{
    data.data = NULL;
    data.len = 0;
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
    this->stream = new NativeStream(net, url);

    stream->setDelegate(this);
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

    this->data.data = (unsigned char *)malloc(len);
    memcpy(this->data.data, data, len);
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

void NativeAssets::Item::onGetContent(const char *data, size_t len)
{
    this->setContent(data, len);
}

void NativeAssets::addToPendingList(Item *item)
{
    struct item_list *il = (struct item_list *)malloc(sizeof(*il));

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

void NativeAssets::pendingListUpdate()
{
    struct item_list *il = pending_list.head, *ilnext;
    while (il != NULL && il->item->state == NativeAssets::Item::ITEM_LOADED) {
        itemReady(il->item, readyArg);

        pending_list.head = il->next;

        if (il->next == NULL) {
            pending_list.foot = NULL;
        }

        ilnext = il->next;
        free(il);

        il = ilnext;
    }
}

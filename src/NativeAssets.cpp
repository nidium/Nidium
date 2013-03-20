#include "NativeAssets.h"

NativeAssets::NativeAssets(readyItem cb, void *arg) :
    itemReady(cb), readyArg(arg)
{
    pending_list.head = NULL;
    pending_list.foot = NULL;
}

NativeAssets::Item::Item(const char *url, FileType t, ape_global *net) :
    fileType(t), state(ITEM_LOADING),
    url(url), net(net), assets(NULL), name(NULL)
{
    data.data = NULL;
    data.len = 0;
}

NativeAssets::Item::~Item()
{
    if (name) {
        free(name);
    }
    if (this->data.data) {
        free(this->data.data);
    }
}

void NativeAssets::Item::download()
{
    NativeStream *stream = new NativeStream(net, url);

    stream->setDelegate(this);
    stream->getContent();
}

void NativeAssets::Item::onGetContent(const char *data, size_t len)
{
    this->setContent(data, len);

    if (assets) {
        assets->pendingListUpdate();
    }
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
    for (struct item_list *il = pending_list.head; il != NULL &&
        il->item->state == NativeAssets::Item::ITEM_LOADED; il = il->next) {

        itemReady(il->item, readyArg);

        pending_list.head = il->next;

        if (il->next == NULL) {
            pending_list.foot = NULL;
        }

        free(il);
    }
}

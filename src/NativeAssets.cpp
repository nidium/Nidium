#include "NativeAssets.h"

NativeAssets::NativeAssets(readyItem cb, void *arg) :
    itemReady(cb), readyArg(arg)
{
    pending_list.head = NULL;
    pending_list.foot = NULL;
}

NativeAssets::Item::Item(const char *url, ape_global *net) :
    state(ITEM_LOADING), url(url), net(net), assets(NULL), name(NULL)
{

}

NativeAssets::Item::~Item()
{
    if (name) {
        free(name);
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
    this->state = ITEM_LOADED;

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

    item->download();
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

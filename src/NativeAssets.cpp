#include "NativeAssets.h"

NativeAssets::NativeAssets(readyItem cb, void *arg) :
    itemReady(cb), pending_list(NULL)
{

}

NativeAssets::Item::Item(const char *url, ape_global *net) :
    state(ITEM_LOADING), url(url), net(net)
{

}

void NativeAssets::Item::download()
{
    NativeStream *stream = new NativeStream(net, url);

    stream->setDelegate(this);
    stream->getContent();
}

void NativeAssets::Item::onGetContent(const char *data, size_t len)
{
    printf("Got the content!!! %ld\n", len);
}

void NativeAssets::addToPendingList(Item *item)
{
    struct item_list *il = (struct item_list *)malloc(sizeof(*il));

    il->item = item;
    il->prev = NULL;
    il->next = pending_list;

    if (pending_list) {
        pending_list->prev = il;
    }

    pending_list = il;

    item->download();
}


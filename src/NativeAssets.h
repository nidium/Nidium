#ifndef nativeassets_h__
#define nativeassets_h__

#include <stdint.h>
#include <stdlib.h>
#include <ape_hash.h>
#include <native_netlib.h>
#include "NativeStream.h"


class NativeAssets
{
    public:

        class Item : public NativeStreamDelegate
        {
            public:
                Item(const char *url, ape_global *net);
                ~Item();
                void download();
                const unsigned char get(size_t *size);

                enum {
                    ITEM_LOADING,
                    ITEM_LOADED
                } state;

            private:
                const char *url;
                ape_global *net;
                void onGetContent(const char *data, size_t len);
        };


        typedef void (*readyItem)(NativeAssets::Item *item, void *arg);

        void addToPendingList(Item *item);
        NativeAssets(readyItem cb, void *arg);
        ~NativeAssets(){};

        readyItem itemReady;

    private:
        struct item_list {
            Item *item;

            struct item_list *next;
            struct item_list *prev;
        };

        struct item_list *pending_list;
};


#endif

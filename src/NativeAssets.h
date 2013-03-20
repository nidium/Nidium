#ifndef nativeassets_h__
#define nativeassets_h__

#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <ape_hash.h>
#include <native_netlib.h>
#include "NativeStream.h"

class NativeAssets
{
    public:

        class Item : public NativeStreamDelegate
        {
            friend class NativeAssets;
            public:
                Item(const char *url, ape_global *net);
                ~Item();
                void download();
                const unsigned char get(size_t *size);

                enum {
                    ITEM_LOADING,
                    ITEM_LOADED
                } state;

                const char *getName() const {
                    return this->name;
                }

                void setName(const char *name) {
                    this->name = strdup(name);
                }

            private:
                const char *url;
                ape_global *net;
                void onGetContent(const char *data, size_t len);
                NativeAssets *assets;
                char *name;
        };


        typedef void (*readyItem)(NativeAssets::Item *item, void *arg);

        void addToPendingList(Item *item);
        
        NativeAssets(readyItem cb, void *arg);
        ~NativeAssets(){};

        readyItem itemReady;
        void *readyArg;

    private:
        struct item_list {
            Item *item;

            struct item_list *next;
        };

        struct {
            struct item_list *head;
            struct item_list *foot;
        } pending_list;

        void pendingListUpdate();
};


#endif

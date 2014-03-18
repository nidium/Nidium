#ifndef nativeassets_h__
#define nativeassets_h__

#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <ape_hash.h>
#include <native_netlib.h>
#include "NativeStream.h"
#include <NativeStreamInterface.h>

class NativeStream;

class NativeAssets
{
    public:

        class Item : public NativeMessages
        {
            friend class NativeAssets;
            public:
                enum FileType {
                    ITEM_UNKNOWN,
                    ITEM_SCRIPT,
                    ITEM_NSS,
                    ITEM_IMG
                } fileType;

                Item(const char *url, FileType t, ape_global *net);
                ~Item();
                void download();
                const unsigned char *get(size_t *size) {
                    if (size != NULL) {
                        *size = this->data.len;
                    }
                    return this->data.data;
                }

                void setContent(const char *data, size_t len, bool async = false);

                enum {
                    ITEM_LOADING,
                    ITEM_LOADED
                } state;

                const char *getName() const {
                    return this->name;
                }
                const char *getTagName() const {
                    return this->tagname;
                }

                void setName(const char *name) {
                    if (this->name != NULL && this->name != name) {
                        free(this->name);
                    }
                    this->name = strdup(name);
                }

                void setTagName(const char *name) {
                    this->tagname = strdup(name);
                }

                NativeBaseStream *stream;
                void onMessage(const NativeSharedMessages::Message &msg);
            private:
                const char *url;
                ape_global *net;
                NativeAssets *assets;
                char *name;
                char *tagname;

                struct {
                    unsigned char *data;
                    size_t len;
                } data;
        };


        typedef void (*readyItem)(NativeAssets::Item *item, void *arg);
        typedef void (*readyAssets)(NativeAssets *assets, void *arg);

        void addToPendingList(Item *item);
        
        NativeAssets(readyItem cb, readyAssets rcb, void *arg);
        ~NativeAssets();

        readyItem itemReady;
        readyAssets assetsReady;
        void *readyArg;
        void pendingListUpdate();
        void endListUpdate(ape_global *net);
    private:
        uint32_t nitems;
        struct item_list {
            Item *item;

            struct item_list *next;
        };

        struct {
            struct item_list *head;
            struct item_list *foot;
        } pending_list;
};


#endif

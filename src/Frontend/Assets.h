/*
   Copyright 2016 Nidium Inc. All rights reserved.
   Use of this source code is governed by a MIT license
   that can be found in the LICENSE file.
*/
#ifndef frontend_assets_h__
#define frontend_assets_h__

#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#include <ape_hash.h>
#include <ape_netlib.h>

#include "IO/Stream.h"
#include "Core/Messages.h"

namespace Nidium {
namespace Frontend {

class Assets
{
public:
    class Item : public Core::Messages
    {
        friend class Assets;

    public:
        enum FileType
        {
            ITEM_UNKNOWN,
            ITEM_SCRIPT,
            ITEM_NSS,
            ITEM_IMG
        } m_FileType;

        Item(const char *url, FileType t, ape_global *net);
        ~Item();
        void download();
        const unsigned char *get(size_t *size)
        {
            if (size != NULL) {
                *size = m_Data.len;
            }
            return m_Data.data;
        }

        void setContent(const char *data, size_t len, bool async = false);

        enum
        {
            ITEM_LOADING,
            ITEM_LOADED
        } m_State;

        const char *getName() const
        {
            return m_Name;
        }
        const char *getTagName() const
        {
            return m_Tagname;
        }

        void setName(const char *name)
        {
            if (m_Name != NULL && m_Name != name) {
                free(m_Name);
            }
            m_Name = strdup(name);
        }

        void setTagName(const char *name)
        {
            m_Tagname = strdup(name);
        }

        IO::Stream *m_Stream;
        void onMessage(const Core::SharedMessages::Message &msg);

    private:
        const char *m_Url;
        ape_global *m_Net;
        Assets *m_Assets;
        char *m_Name;
        char *m_Tagname;

        struct
        {
            unsigned char *data;
            size_t len;
        } m_Data;
    };

    typedef void (*readyItem)(Assets::Item *item, void *arg);
    typedef void (*readyAssets)(Assets *m_Assets, void *arg);

    void addToPendingList(Item *item);

    Assets(readyItem cb, readyAssets rcb, void *arg);
    ~Assets();

    readyItem m_ItemReady;
    readyAssets m_AssetsReady;
    void *m_ReadyArg;
    void pendingListUpdate();
    void endListUpdate(ape_global *net);

private:
    uint32_t m_Nitems;
    struct item_list
    {
        Item *item;

        struct item_list *next;
    };

    struct
    {
        struct item_list *head;
        struct item_list *foot;
    } m_Pending_list;
};

} // namespace Frontend
} // namespace Nidium

#endif

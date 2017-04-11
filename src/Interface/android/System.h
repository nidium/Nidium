/*
   Copyright 2016 Nidium Inc. All rights reserved.
   Use of this source code is governed by a MIT license
   that can be found in the LICENSE file.
*/
#ifndef interface_android_system_h__
#define interface_android_system_h__

#include "SystemInterface.h"
#include <jnipp.h>

namespace Nidium {
namespace Interface {

class System : public SystemInterface
{
public:
    System(JNIEnv *env, jobject cx);
    ~System();
    float backingStorePixelRatio() override;
    const char *getCacheDirectory() override;
    const char *getEmbedDirectory() override;
    const char *getUserDirectory() override;
    void alert(const char *message, AlertType type = ALERT_INFO) override;
    const char *cwd() override;
    const char *getLanguage() override;
    void sendNotification(const char *title, const char *content, bool sound) override;
    const char *execute(const char *cmd) override;
    void print(const char *buf) override;

    int getSurfaceWidth();
    int getSurfaceHeight();

private:
    jnipp::GlobalRef<jnipp::Object> m_Nidroid;
    jnipp::GlobalRef<jnipp::Class> m_NidroidClass;

    jnipp::Env::Scope *m_JNIScope = nullptr;

    float m_PixelRatio      = 1;
    char *m_Language        = nullptr;
    char *m_UserDirectory   = nullptr;
    char *m_CacheDirectory  = nullptr;
    char *m_EmbedDirectory  = nullptr;
};

} // namespace Interface
} // namespace Nidium

#endif

/*
   Copyright 2016 Nidium Inc. All rights reserved.
   Use of this source code is governed by a MIT license
   that can be found in the LICENSE file.
*/
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>
#include <unistd.h>
#include <pwd.h>
#include <locale.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/param.h>
#include <libgen.h>
#include <string>

#include <SDL.h>

#include "System.h"
#include "Macros.h"


namespace Nidium {
namespace Interface {

System::System(JNIEnv *env, jobject nidroid) 
{
    m_JNIScope = new jnipp::Env::Scope(env);
    jnipp::Ref<jnipp::Object> nidroidRef(nidroid);
    
    m_Nidroid.set(nidroidRef);
    m_NidroidClass.set(nidroidRef->getClass());

    jnipp::StaticMethod<jdouble> getPixelRatio(m_NidroidClass, "getPixelRatio", "()D");
    jnipp::StaticMethod<jnipp::String> getUserDirectory(m_NidroidClass, "getUserDirectory", "()Ljava/lang/String;");
    jnipp::StaticMethod<jnipp::String> getLanguage(m_NidroidClass, "getLanguage", "()Ljava/lang/String;");
    jnipp::Method<jnipp::String> getCacheDirectory(m_NidroidClass, "getCacheDirectory", "()Ljava/lang/String;");

    m_PixelRatio = getPixelRatio();
    m_UserDirectory = getUserDirectory()->str();
    m_CacheDirectory = getCacheDirectory(m_Nidroid)->str();
    m_Language = getLanguage()->str();

    static const char embedDir[] = "/nidium/Embed/";
    size_t size = (strlen(m_UserDirectory) + strlen(embedDir) + 1);
    m_EmbedDirectory = (char *)malloc(sizeof(char) * size);
    snprintf(m_EmbedDirectory, size, "%s%s", m_UserDirectory, embedDir);
}

System::~System()
{
    delete m_JNIScope;
    free(m_CacheDirectory);
    free(m_UserDirectory);
}

float System::backingStorePixelRatio()
{
    return m_PixelRatio;
}

const char *System::getEmbedDirectory()
{
    return m_EmbedDirectory;
}

const char *System::getUserDirectory()
{
    return m_UserDirectory;
}

const char *System::getCacheDirectory()
{
    return m_CacheDirectory;
}

void System::alert(const char *message, AlertType type)
{
    jnipp::LocalRef<jnipp::String> jMessage(jnipp::String::create(message));
    static jnipp::Method<void, jnipp::String, jint> alert(m_NidroidClass, "alert", "(Ljava/lang/String;I)V");

    alert(m_Nidroid, jMessage, type);
}

int System::getSurfaceWidth()
{
    static jnipp::Method<jint> getSurfaceWidth(m_NidroidClass, "getSurfaceWidth", "()I");
    return getSurfaceWidth(m_Nidroid);
}

int System::getSurfaceHeight()
{
    static jnipp::Method<jint> getSurfaceHeight(m_NidroidClass, "getSurfaceHeight", "()I");
    return getSurfaceHeight(m_Nidroid);
}

const char *System::cwd()
{
    static char dir[MAXPATHLEN];

    getcwd(dir, MAXPATHLEN);

    return dir;
}
const char *System::getLanguage()
{
    return m_Language;
}

void System::sendNotification(const char *title,
                              const char *content,
                              bool sound)
{
    jnipp::LocalRef<jnipp::String> jTitle(jnipp::String::create(title));
    jnipp::LocalRef<jnipp::String> jContent(jnipp::String::create(content));
    static jnipp::Method<void, jnipp::String, jnipp::String> notify(m_NidroidClass, "notify", "(Ljava/lang/String;Ljava/lang/String;)V");

    notify(m_Nidroid, jTitle, jContent);
}

const char *System::execute(const char *cmd)
{
    return nullptr;
}

void System::print(const char *buf)
{
    __android_log_print(ANDROID_LOG_INFO, "Nidium", "%s", buf);
}

void System::showVirtualKeyboard(int flags)
{
    static jnipp::StaticMethod<void, jint> SetKeyboardOptions(m_NidroidClass, "SetKeyboardOptions", "(I)V");

    SetKeyboardOptions(flags);

    SDL_StartTextInput();
}

void System::hideVirtualKeyboard()
{
    SDL_StopTextInput();
}

void System::stopScrolling()
{
    jnipp::Method<void> stopScrolling(m_NidroidClass, "stopScrolling", "()V");

    stopScrolling(m_Nidroid);
}

} // namespace Interface
} // namespace Nidium

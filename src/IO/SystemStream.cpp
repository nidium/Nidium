#include "SystemStream.h"

#include <Interface/SystemInterface.h>
#include <Interface/UIInterface.h>
#include <Frontend/NML.h>
#include <Frontend/Context.h>

namespace Nidium {
    namespace Interface {
        class UIInterface;
        extern UIInterface *__NidiumUI;
    }

namespace IO {
// {{{ PrivateStream Implementation
const char *PrivateStream::m_BaseDir = nullptr;

const char *PrivateStream::GetBaseDir()
{
    if (m_BaseDir == nullptr) {
        m_BaseDir = Interface::SystemInterface::GetInstance()->getCacheDirectory();

        Frontend::NML *nml = Nidium::Interface::__NidiumUI->getNidiumContext()->getNML();
        if (nml) {
            const char *appDir = nml->getIdentifier();
            char *destDir = nullptr;

            if (asprintf(&destDir, "%s%s/", m_BaseDir, appDir) == -1) {
                return nullptr;
            }

            // Change dots to slashes, to avoid a cluttered directory structure
            for (int i = strlen(m_BaseDir) - 1; i < strlen(destDir); i++) {
                if (destDir[i] == '.') {
                    destDir[i] = '/';
                }
            }

            Core::Path::Makedirs(destDir);

            m_BaseDir = destDir;
        }
    }

    return m_BaseDir;
}
// }}}

// {{{ UserStream Implementation
const char *UserStream::GetBaseDir() {
    return Interface::SystemInterface::GetInstance()->getUserDirectory();
}
// }}}
} // namespace IO
} // namespace Nidium


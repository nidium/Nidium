/*
   Copyright 2017 Nidium Inc. All rights reserved.
   Use of this source code is governed by a MIT license
   that can be found in the LICENSE file.
*/

#include "System.h"

#include <Port/MSWindows.h>

#ifdef _MSC_VER
//Duh
#include <Shlobj.h>
#endif

namespace Nidium {
namespace Interface {

System::System(){};

const char* System::getUserDirectory() {
   // caller must free memory after receiving result
   TCHAR *path = new TCHAR[128];
   SHGetSpecialFolderPath(NULL, path, CSIDL_PERSONAL, FALSE);
    return path;
};

} // namespace Interface
} // namespace Nidium


#include "System.h"

System::System(){}

const char* System::getUserDirectory() {
	// caller must free memory after receiving result
	TCHAR *path = new TCHAR[128];
	SHGetSpecialFolderPath(NULL, 
				           path,
						   CSIDL_PERSONAL, 
						   FALSE);
    return path;
}


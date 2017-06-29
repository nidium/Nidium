/*
   Copyright 2016 Nidium Inc. All rights reserved.
   Use of this source code is governed by a MIT license
   that can be found in the LICENSE file.
*/

#include "System/OS.h"

#include <string>
#include <unistd.h>

namespace Nidium {
namespace System {

int OS::getTempFd()
{
	const char *tmpPath = this->getTempPath();
	if (tmpPath == nullptr) {
		return -1;
	}

	char *tmpfname = strdup((std::string(tmpPath) + "nidiumtmp.XXXXXX").c_str());
	int fd = mkstemp(tmpfname);
	free(tmpfname);

	return fd;
}

}}
/*
   Copyright 2016 Nidium Inc. All rights reserved.
   Use of this source code is governed by a MIT license
   that can be found in the LICENSE file.
*/

function dummyFilename() {
	return __filename;
}

if (typeof module != "undefined") {
    module.exports = {
        dummyFilename: dummyFilename
    }
}

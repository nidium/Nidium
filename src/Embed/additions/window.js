/*
   Copyright 2017 Nidium Inc. All rights reserved.
   Use of this source code is governed by a MIT license
   that can be found in the LICENSE file.
*/

window.location = {
    href: "nidium:app", // XXX : Get the app name from the NML ?
}

window.location[Symbol.toPrimitive] = function(hint) {
    return this.href;
}

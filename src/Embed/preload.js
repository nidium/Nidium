/*
   Copyright 2016 Nidium Inc. All rights reserved.
   Use of this source code is governed by a MIT license
   that can be found in the LICENSE file.
*/

function __nidiumPreload(options) {O
    if (options.html5) {
        load("embed://html5.js");
    }
    load("embed://CanvasAdditions.js");
    load("embed://NMLAdditions.js");
    load("embed://AnimationBlock.js");
    load("embed://Elements.js");
}

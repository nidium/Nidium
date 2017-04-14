/*
   Copyright 2016 Nidium Inc. All rights reserved.
   Use of this source code is governed by a MIT license
   that can be found in the LICENSE file.
*/

function __nidiumPreload(options, lst) {

    if (options.html5) {
        load("embed://html5.js");
    }
    load("embed://CanvasAdditions.js");
    load("embed://ContextAdditions.js");
    load("embed://NMLAdditions.js");
    load("embed://AnimationBlock.js");
    load("embed://HTTPAdditions.js");
    
    let rdebug = require("RemoteDebug.js");

    rdebug.run(9223, (options.remotedebug == "true")
        ? "0.0.0.0"
        : "127.0.0.1");

    document.canvas.inject(lst);
}

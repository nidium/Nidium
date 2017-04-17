/*
 * Copyright 2017 Nidium Inc. All rights reserved.
 * Use of this source code is governed by a MIT license
 * that can be found in the LICENSE file.
 */

/**
 * Defer creation of the layout once all assets are ready
 */
Object.defineProperty(window, "_onready", {
    "configurable": false,
    "writable": false,
    "value": function(lst) {
        document.canvas.inject(lst);
    }
});

/*
 * Preload and class extends
 */
function __nidiumPreload(options, lst) {
    if (options.html5) load("embed://html5.js");

    /**
     * Additions
     */
    load("embed://additions/Canvas.js");
    load("embed://additions/NML.js");
    load("embed://additions/HTTP.js");
    load("embed://additions/document.js");
    /*
     * Native UI Controls and NML tags
     */
    load("embed://framework/index.js");
    
    /**
     * Misc
     */
    load("embed://AnimationBlock.js");
}

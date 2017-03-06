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

    /**
     * Misc
     */
    load("embed://AnimationBlock.js");
}

if (0) {
    var CreateCatchAllProxy = function(base = {}) {
        return new Proxy(Object.assign(base, {[Symbol.toPrimitive]: () => 'hey'}), {
            get: (target, property, value, rcv) => {

                if (!(property in target)) {
                    target[property] = CreateCatchAllProxy({
                        __accessor: target.__accessor ? `${target.__accessor}.${property}` : property
                    });
                }

                return target[property];
            },

            set: (target, property, rcv) => {
                throw Error("Assignation is forbiden");
            }
        })
    }

    var dynamic = CreateCatchAllProxy();

    with (dynamic.foo) {
        console.log("value", dynamic.foo.bar.yo.asd.asd.__accessor);
    }
}

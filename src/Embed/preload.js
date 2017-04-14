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

    document.canvas.inject(lst);
}

if (0) {
var CreateCatchAllProxy = function(base = {}) {
    return new Proxy(Object.assign(base, {[Symbol.toPrimitive]: () => 'hey'}), {
        get: (target, property, value, rcv) => {

            if (!(property in target)) {
                target[property] = CreateCatchAllProxy({__accessor: target.__accessor ? `${target.__accessor}.${property}` : property});
            }

            return target[property];
        },

        set: (target, property, rcv) => {
            throw Error("Assignation is forbiden");
        }
    })
}

var dynamic = CreateCatchAllProxy();

with(dynamic.foo) {
    console.log("value", dynamic.foo.bar.yo.asd.asd.__accessor);
}
}
/*
 * Copyright 2017 Nidium Inc. All rights reserved.
 * Use of this source code is governed by a MIT license
 * that can be found in the LICENSE file.
 */

{
    const VM            = require("VM");
    const Elements      = require("Elements");
    const s_ShadowRoot  = require("../../Symbols.js").ElementShadowRoot;

    Elements.script = class extends Elements.Node {
        constructor(attributes) {
            super(attributes);
            this.hide();
            if (attributes.src) {
                this.data = Elements.Loader(attributes);
            }
        }

        createTree(children) {
            let code = this.data;
            if (!code) {
                code = children[0].text;
            }
            VM.run(code, {"scope": this[s_ShadowRoot].getJSScope()})
        }
    }
}

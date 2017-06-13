/*
 * Copyright 2017 Nidium Inc. All rights reserved.
 * Use of this source code is governed by a MIT license
 * that can be found in the LICENSE file.
 */

{
    const VM            = require("VM");
    const Elements      = require("Elements");
    const s_ShadowRoot  = require("../../Symbols.js").ElementShadowRoot;

    class NSS {
        constructor(shadowRoot, nss, filename) {
            this.shadowRoot = shadowRoot;
            this.filename   = filename;
            this.nss        = nss;

            this.shadowRoot.addNSS(this);
        }

        eval() {
            return VM.run(`(function() { return {${this.nss}} })()`, {
                "scope": this.shadowRoot.getJSScope(),
                "filename": this.filename
            })
        }
    }

    Elements.nss = class extends Elements.Node {
        constructor(attributes) {
            super(attributes);

            this.hide();

            this.filename = "inline nss";

            if (attributes.src) {
                this.data = Elements.Loader(attributes);
                this.filename = attributes.src;
            }
        }

        createTree(children) {
            let nss = this.data || children[0].text;
            this.nss = new NSS(this[s_ShadowRoot], nss, this.filename);
        }
    }
}

/*
 * Copyright 2017 Nidium Inc. All rights reserved.
 * Use of this source code is governed by a MIT license
 * that can be found in the LICENSE file.
 */

{
    const Elements = require("Elements");

    Elements.comment = class extends Elements.Node {
        constructor(data) {
            super({});
            this.hide();
            this.nodeValue = data;
        }

        get nodeType() {
            return Elements.NodeType.COMMENT_NODE;
        }

        get nodevalue() {
            return this._comment;
        }

        set nodeValue(comment) {
            this._comment = comment;
        }
    }
}

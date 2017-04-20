/*
 * Copyright 2017 Nidium Inc. All rights reserved.
 * Use of this source code is governed by a MIT license
 * that can be found in the LICENSE file.
 */

{
    const Elements = require("Elements");

    Elements.slot = class extends Elements.Node {
        constructor(attr) {
            super(attr);
            this.flexGrow = 1;
        }

    }
}

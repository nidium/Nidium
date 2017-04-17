/**
 * Copyright 2017 Nidium Inc. All rights reserved.
 * Use of this source code is governed by a MIT license
 * that can be found in the LICENSE file.
 */

{
    const Elements = require("Elements");

    Elements.view = class extends Elements.Element {
        constructor(attributes) {
            super(attributes);
            this.style.position = "absolute";
            this.style.left = 0;
            this.style.top = 0;
            this.style.right = 0;
            this.style.bottom = 0;
        }
    }
}
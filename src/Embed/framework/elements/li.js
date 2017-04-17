/**
 * Copyright 2017 Nidium Inc. All rights reserved.
 * Use of this source code is governed by a MIT license
 * that can be found in the LICENSE file.
 */

{
    const Elements = require("Elements");

    Elements.li = class extends Elements.Element {
        constructor(attributes) {
            super(attributes);

            this.style.flexDirection = "row";
            this.style.justifyContent = "space-between";
            this.style.alignItems = "center";
            this.style.flexWrap = "nowrap";
        }

        paint(ctx) {
            super.paint(ctx)
        }
    }
}
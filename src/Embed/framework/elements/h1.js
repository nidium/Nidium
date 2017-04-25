/**
 * Copyright 2017 Nidium Inc. All rights reserved.
 * Use of this source code is governed by a MIT license
 * that can be found in the LICENSE file.
 */

{
    const Elements = require("Elements");
    const { StyleContainer, ElementStyle } = require("ElementsStyles");

    Elements.h1 = class extends Elements.Element {
        constructor(attributes) {
            super(attributes);

            this.setDefaultStyle({
                fontSize: 36,
                lineHeight: 36,
                width: "100%",
                textAlign: "left",
                minHeight: this.style.lineHeight || 20,
                marginTop: 6,
                marginBottom: 6
            });

            /* FROZEN ------ */
        }
    }

    ElementStyle.Inherit(Elements.h1);
}
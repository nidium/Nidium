/**
 * Copyright 2017 Nidium Inc. All rights reserved.
 * Use of this source code is governed by a MIT license
 * that can be found in the LICENSE file.
 */

{
    const Elements = require("Elements");
    const { StyleContainer, ElementStyle } = require("ElementsStyles");

    Elements.p = class extends Elements.Element {
        constructor(attributes) {
            super(attributes);

            this.setDefaultStyle({
                fontSize : 15,
                lineHeight : 20
            });

            this.style.minHeight = this.style.lineHeight || 20;
            this.style.textAlign = "left";
            this.style.flexWrap = "nowrap";
            this.style.width = "100%";
            this.style.marginTop = 6;
            this.style.marginBottom = 6;
        }
    }

    ElementStyle.Inherit(Elements.p);
}
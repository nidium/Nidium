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

            this.style.flexDirection = "row";
            this.style.flexWrap = "nowrap";
            this.style.width = "100%";
            this.style.textAlign = "left";
            this.style.minHeight = this.style.lineHeight || 20;
            this.style.marginTop = 6;
            this.style.marginBottom = 6;

            this.inherit.fontSize = this.inherit.fontSize || 40;
            this.inherit.lineHeight = this.inherit.lineHeight || 40;
        }
    }

    ElementStyle.Inherit(Elements.h1);
}
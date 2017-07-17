/**
 * Copyright 2017 Nidium Inc. All rights reserved.
 * Use of this source code is governed by a MIT license
 * that can be found in the LICENSE file.
 */

{
    const Elements = require("Elements");
    const { StyleContainer, ElementStyle } = require("ElementsStyles");

    Elements.row = class extends Elements.Element {
        constructor(a={}) {
            super(a);

            var size = a.size || 1;

            this.style.flexGrow = size;
            this.style.flexDirection = "row";

            this.style.alignItems = a.valign || "stretch";
            this.style.justifyContent = a.align || "flex-start";

            this.responsive = {
                xs: a.xs || size,
                sm: a.sm || size,
                md: a.md || size,
                lg: a.lg || size,
                xl: a.xl || size
            };
        }
    }

    ElementStyle.Inherit(Elements.row);
}
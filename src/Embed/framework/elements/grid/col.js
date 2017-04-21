/**
 * Copyright 2017 Nidium Inc. All rights reserved.
 * Use of this source code is governed by a MIT license
 * that can be found in the LICENSE file.
 */

{
    const Elements = require("Elements");
    const responsive = require("../../core/responsive.js");

    const { StyleContainer, ElementStyle } = require("ElementsStyles");

    Elements.col = class extends Elements.Element {
        constructor(a) {
            super(a);

            var a = a || {},
                size = a.size || 1;

            this.style.flexGrow = size
            this.style.flexDirection = 'column';
            this.style.backgroundColor = "green";

            this.responsive = {
                xs : a.xs || size,
                sm : a.sm || size,
                md : a.md || size,
                lg : a.lg || size,
                xl : a.xl || size,
            };
        }

        paint(ctx, w, h) {
            super.paint(ctx, w, h);

            var r = responsive(),
                size = this.responsive[r];

            if (size=="hidden") {
                console.log("hidde")
                this.width=0;
                this.minWidth=0;
                this.maxWidth=0;
                this.hide();
            } else {
                this.style.flexGrow = size;
                console.log(r, size)
                this.show();
            }

        }
    }

    ElementStyle.Inherit(Elements.col);
}
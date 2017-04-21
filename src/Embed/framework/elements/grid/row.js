/**
 * Copyright 2017 Nidium Inc. All rights reserved.
 * Use of this source code is governed by a MIT license
 * that can be found in the LICENSE file.
 */

{
    const Elements = require("Elements");
    const { StyleContainer, ElementStyle } = require("ElementsStyles");

    Elements.row = class extends Elements.Element {
        constructor(a) {
            super(a);

            this.style.flexGrow = a && a.size ? a.size : 1;
            this.style.flexDirection = "row";
            this.style.backgroundColor = "blue";
        }
    }

    ElementStyle.Inherit(Elements.row);
}
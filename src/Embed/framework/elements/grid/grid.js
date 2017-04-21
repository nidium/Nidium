/**
 * Copyright 2017 Nidium Inc. All rights reserved.
 * Use of this source code is governed by a MIT license
 * that can be found in the LICENSE file.
 */

{
    const Elements = require("Elements");
    const { StyleContainer, ElementStyle } = require("ElementsStyles");

    Elements.grid = class extends Elements.Element {
        constructor(attributes) {
            super(attributes);

            this.flexGrow = 1;
            this.style.flexDirection = "column";
            this.style.justifyContent = "space-between";
            this.style.alignItems = "stretch";
            this.style.flexWrap = "nowrap";
            this.style.backgroundColor = "red";
        }
    }

    ElementStyle.Inherit(Elements.grid);
}
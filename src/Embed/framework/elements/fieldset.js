/**
 * Copyright 2017 Nidium Inc. All rights reserved.
 * Use of this source code is governed by a MIT license
 * that can be found in the LICENSE file.
 */

{
    const Elements = require("Elements");
    const { StyleContainer, ElementStyle } = require("ElementsStyles");
    
    Elements.fieldset = class extends Elements.Element {
        constructor(attributes) {
            super(attributes);

            this.style.flexGrow = 1;
            this.style.flexDirection = "row";
            this.style.justifyContent = "flex-start";
            this.style.alignItems = "center";
        }
    }

    ElementStyle.Inherit(Elements.fieldset);
}

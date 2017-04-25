/**
 * Copyright 2017 Nidium Inc. All rights reserved.
 * Use of this source code is governed by a MIT license
 * that can be found in the LICENSE file.
 */

{
    const Elements = require("Elements");
    const { StyleContainer, ElementStyle } = require("ElementsStyles");
    
    Elements.label = class extends Elements.Element {
        constructor(attributes) {
            super(attributes);

            this.style.flexGrow = 1;
        }

        onload() {
            let prev = this.prevSibling;

            if (prev.name() == 'radio' ) {

                if (prev.disabled) {
                    this.style.color = "#c0c0c0";
                }

                this.cursor = prev.cursor;
                this.on("click", () => {
                    prev.emit("click", {});
                });

            }

        }
    }

    ElementStyle.Inherit(Elements.label);
}

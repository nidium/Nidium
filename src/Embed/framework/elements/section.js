/**
 * Copyright 2017 Nidium Inc. All rights reserved.
 * Use of this source code is governed by a MIT license
 * that can be found in the LICENSE file.
 */

{
    const Elements = require("Elements");
    const { StyleContainer, ElementStyle } = require("ElementsStyles");
    
    Elements.section = class extends Elements.Element {
        constructor(attributes) {
            super(attributes);
        }
    }

    ElementStyle.Inherit(Elements.section);
}

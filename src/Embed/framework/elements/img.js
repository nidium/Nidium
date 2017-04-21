/**
 * Copyright 2017 Nidium Inc. All rights reserved.
 * Use of this source code is governed by a MIT license
 * that can be found in the LICENSE file.
 */

{
    const Elements = require("Elements");
    const { StyleContainer, ElementStyle } = require("ElementsStyles");

    Elements.img = class extends Elements.Element {
        constructor(attributes={}) {
            super(attributes);
            this.src = attributes.src;
            this._loaded = false;

            if (attributes.width) {
                this.style.width = attributes.width;
            }
            if (attributes.height) {
                this.style.height = attributes.height;
            }
        }

        set src(value) {
            if (!value) return;

            this._src = value;
            this._img = new Image();
            this._img.src = value;

            this._img.onload = () => {
                var w = this._img.width;
                var h = this._img.height;

                var ar = w/h;

                this._loaded = true;

                if (this.attributes.onload) {
                    this.attributes.onload.call(this, this._img);
                }

                if (this.style.width && !this.style.height ||
                    !this.style.width && this.style.height) {
                    this.style.aspectRatio = ar;
                }

                this.requestPaint();
            }
        }

        get src() {
            return this._src;
        }

        paint(ctx, width, height) {
            if (this._loaded) {
                ctx.drawImage(this._img, 0, 0, width, height);
            }
        }

    }

    ElementStyle.Inherit(Elements.img);
}

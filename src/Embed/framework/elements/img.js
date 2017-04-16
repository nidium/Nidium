/**
 * Copyright 2017 Nidium Inc. All rights reserved.
 * Use of this source code is governed by a MIT license
 * that can be found in the LICENSE file.
 */

{
    const Elements = require("Elements");

    Elements.img = class extends Elements.Element {
        constructor(attributes) {
            super(attributes);
            this.src = attributes.src;
            this._loaded = false;
        }

        set src(value) {
            if (!value) return;
            console.log("set src", value)
            this._src = value;
            this._img = new Image();
            this._img.src = value;

            this._img.onload = () => {
                this._loaded = true;

                if (this.attributes.onload) {
                    this.attributes.onload.call(this, this._img);
                }

                this.setSize(this._img.width, this._img.height);
                this.requestPaint();
            }
        }

        get src() {
            return this._src;
        }

        paint(ctx) {
            ctx.imageSmoothingEnabled = true;

            if (this._loaded) {
                ctx.drawImage(this._img, 0, 0, this.width, this.height);
            }
        }

    }
}
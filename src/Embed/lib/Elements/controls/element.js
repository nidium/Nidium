/**
 * Copyright 2017 Nidium Inc. All rights reserved.
 * Use of this source code is governed by a MIT license
 * that can be found in the LICENSE file.
 */

{
    const StyleContainer = require("./../../StyleContainer.js");

    const Elements = require("Elements");

    Elements.Element = class extends Elements.Node {
        constructor(attributes = {}) {
            super(attributes);

            if (attributes.opacity !== undefined) {
                this.opacity = attributes.opacity;
            }

            this.style = new StyleContainer(this);

            //this.onload = this.onpaint;
            this.onresize = this.onpaint;
        }

        onload() {
            this._ctx = this.getContext("2d");
        }

        onpaint() {
            if (!this._ctx) return;

            this._ctx.save();
            this.clear();
            this.paint(this._ctx);
            this._ctx.restore();
        }

        ctx2d() {
            return this.getContext("2d");
        }

        getContext(type="2d") {
            return Canvas.prototype.getContext.call(this, type);
        }

        paint(ctx) {
            this.style.paint(ctx);
        }
    }

    Elements.element = class extends Elements.Element { }
    Elements.none = Elements.Element;
}

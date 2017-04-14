/**
 * Copyright 2017 Nidium Inc. All rights reserved.
 * Use of this source code is governed by a MIT license
 * that can be found in the LICENSE file.
 */

{
    const Elements = require("Elements");

    const s_NodeText = Symbol("NodeText");
    const s_FnParentWidth = Symbol("FunctionTextNodeParentWidth");

    Elements.textnode = class extends Elements.Element {

        constructor(textValue) {
            super({"width": 1, "height": 1});
            this.nodeValue   = textValue;
            this.fluidHeight = false;
            this.fluidWidth  = false;
        }

        cloneNode(deep = true, shadowRoot=this[s_ShadowRoot]) {
            return Elements.Create(this.name(), this.nodeValue, shadowRoot);
        }

        getNMLContent() {
            return this.nodeValue;
        }

        set textContent(value) {
            this.nodeValue = value;
        }

        allowsChild() {
            return false;
        }

        get nodeValue() {
            return this[s_NodeText];
        }

        set nodeValue(textValue) {
            this[s_NodeText] = textValue.trim();
            this.requestPaint();
            this.fireEvent("nodeValueChanged", textValue);
        }

        get nodeType() {
            return Node.TEXT_NODE;
        }

        [s_FnParentWidth](el=this) {
            let p = Canvas.prototype.getParent.call(el);
            if (!p) {
                return window.innerWidth;
            }

            // If the parent has an adaptive width,
            // keep searching for the maximum size
            if (p.fluidWidth || p._fixStaticRight /* p.staticRight */) {
                return this[s_FnParentWidth](p);
            }

            return p.width;
        }

        paint(ctx) {
            super.paint(ctx);
            let p = Canvas.prototype.getParent.call(this);

            let maxWidth    = this[s_FnParentWidth]();
            let actualWidth = 1;

            // FIXME : Get these values from inherited styles
            let fontSize    = 15;
            let lineHeight  = 20;
            let color       = "black";
            let offset      = Math.ceil(lineHeight/2);

            ctx.fontSize        = fontSize;
            ctx.fillStyle       = color;
            ctx.textBaseline    = "middle";

            let data = ctx.breakText(this.nodeValue, maxWidth);

            for (var i = 0; i < data.lines.length; i++) {
                let tmp = ctx.measureText(data.lines[i])
                if (tmp.width > actualWidth) actualWidth = tmp.width;

                ctx.fillText(data.lines[i], 0, i * lineHeight + offset);
            }

            this.width  = actualWidth;
            this.height = lineHeight * data.lines.length;
        }
    }
}

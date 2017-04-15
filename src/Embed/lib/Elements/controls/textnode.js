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
            return Elements.NodeType.TEXT_NODE;
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

            let parentWidth = this[s_FnParentWidth]();
            let actualWidth = 1;

            let fontSize    = this.inherit.fontSize || 15;
            let lineHeight  = this.inherit.lineHeight || 20;
            let color       = this.inherit.color || "#000000";

            let offset      = Math.ceil(lineHeight/2);

            ctx.fontSize        = fontSize;
            ctx.fillStyle       = color;
            ctx.textBaseline    = "middle";

            let data = ctx.breakText(this.nodeValue, parentWidth);

            var ox = 0;

            for (var i = 0; i<data.lines.length; i++) {
                let w = ctx.measureText(data.lines[i]).width;

                if (w > actualWidth) actualWidth = w;

                if (this.inherit.textAlign == "center") {
                    ox = (parentWidth-w)*0.5;
                }
                if (this.inherit.textAlign == "right") {
                    ox = (parentWidth-w);
                }

                ctx.fillText(data.lines[i], ox, i*lineHeight + offset);
            }

            if (p.fluidWidth || p._fixStaticRight) {
                this.width  = actualWidth ;
            } else {
                this.width  = parentWidth;
            }

            this.height = lineHeight * data.lines.length;
        }
    }
}

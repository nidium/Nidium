/**
 * Copyright 2017 Nidium Inc. All rights reserved.
 * Use of this source code is governed by a MIT license
 * that can be found in the LICENSE file.
 */

{
    const Elements = require("Elements");

    const s_NodeText = Symbol("NodeText");
    const { StyleContainer, ElementStyle } = require("ElementsStyles");

    Elements.textnode = class extends Elements.Element {
        constructor(textValue) {
            super();
            this.nodeValue = textValue;
            this.style.flexGrow = 1;
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
            if (textValue == this.nodeValue) return this;
            this[s_NodeText] = textValue.trim();
            this.requestPaint();
            this.fireEvent("nodeValueChanged", textValue);
        }

        get nodeType() {
            return Elements.NodeType.TEXT_NODE;
        }

        paint(ctx, width, height) {
            super.paint(ctx, width, height);

            if (!this.nodeValue) return false;

            let actualWidth = 1;

            let fontSize    = this.inherit.fontSize || 15;
            let lineHeight  = this.inherit.lineHeight || 20;
            let color       = this.inherit.color || "#000000";

            let offset      = Math.ceil(lineHeight/2);

            ctx.fontSize        = fontSize;
            ctx.fillStyle       = color;
            ctx.textBaseline    = "middle";

            let data = ctx.breakText(this.nodeValue, width);
            this.height = lineHeight * data.lines.length;

            var ox = 0;

            for (var i = 0; i<data.lines.length; i++) {
                let w = ctx.measureText(data.lines[i]).width;

                if (w > actualWidth) actualWidth = w;

                if (this.inherit.textAlign == "center") {
                    ox = (width-w)*0.5;
                }
                if (this.inherit.textAlign == "right") {
                    ox = (width-w);
                }

                ctx.fillText(data.lines[i], ox, i*lineHeight + offset);
            }
        }
    }

    ElementStyle.Inherit(Elements.textnode);
}

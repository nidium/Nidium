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
            this.flexGrow = 1;
            this.style.minWidth = 1;
            this.style.minHeight = 1;

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

            var p = this.getParent();
            var dim = p.getDimensions();

            let fontSize    = (this.inherit.fontSize || this.style.fontSize) || 15;
            let lineHeight  = (this.inherit.lineHeight || this.style.lineHeight) || 20;
            let color       = (this.inherit.color || this.style.color) || "#000000";

            let offset      = Math.ceil(lineHeight/2);

            ctx.fontSize        = fontSize;
            ctx.fillStyle       = color;
            ctx.textBaseline    = "middle";

            let data = ctx.breakText(this.nodeValue, dim.width-30);
            this.height = lineHeight * data.lines.length;

            var ox = 0;
            var w = 0;

            for (var i = 0; i<data.lines.length; i++) {
                w = ctx.measureText(data.lines[i]).width;

                if (this.style.textAlign == "center") {
                    ox = (width-w)*0.5;
                }
                if (this.style.textAlign == "right") {
                    ox = (width-w);
                }

                ctx.fillText(data.lines[i], ox, i*lineHeight + offset);
            }

            //if (data.lines.length==1) this.style.minWidth = w;
        }
    }

    ElementStyle.Inherit(Elements.textnode);
}

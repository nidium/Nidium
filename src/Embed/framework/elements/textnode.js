/**
 * Copyright 2017 Nidium Inc. All rights reserved.
 * Use of this source code is governed by a MIT license
 * that can be found in the LICENSE file.
 */

{
    const Elements = require("Elements");

    const s_NodeText = Symbol("NodeText");
    const { StyleContainer, ElementStyle } = require("ElementsStyles");

    document.canvas.inherit._Style_fontFamily = "Roboto Regular";
    document.canvas.inherit._Style_fontSize = 15;

    Elements.textnode = class extends Elements.Element {
        constructor(textValue) {
            super();
            this.nodeValue = textValue;
            this._textData = null;
        }

        cloneNode(deep = true, shadowRoot=this[s_ShadowRoot]) {
            return Elements.Create(this.name(), this.nodeValue, shadowRoot);
        }

        getNMLContent() {
            return this.nodeValue;
        }

        computeSelfSize() {
            if (!this.nodeValue) return;
 
            setTimeout(() => {

                let {width, height} = this.getDimensions(true);

                /* Use document context as we don't have a self context yet */
                var ctx = document.canvas.getContext("2d");
                let fontSize    = this.style.fontSize || 20;
                let lineHeight  = this.style.lineHeight || 20;

                ctx.save();
                    ctx.fontSize     = fontSize;
                    ctx.textBaseline = "middle";
                    this._textData   = ctx.breakText(this.nodeValue, width);
                    this.height      = lineHeight * this._textData.lines.length;

                    if ( this.getParent().name() == "label") {
                        console.log(width, height, this.height, lineHeight, this._textData.lines.length)
                        this.requestPaint();
                    }
                ctx.restore();

            }, 1);
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
            this.computeSelfSize();
            this.requestPaint();
            this.fireEvent("nodeValueChanged", textValue);
        }

        get nodeType() {
            return Elements.NodeType.TEXT_NODE;
        }

        paint(ctx, width, height) {
            super.paint(ctx, width, height);

            if (!this.nodeValue || !this._textData) return false;

            var p = this.getParent();
            var dim = p.getDimensions();

            let fontSize    = this.style.fontSize || 20;
            let lineHeight  = this.style.lineHeight || 20;
            let color       = this.style.color || "#000000";

            let offset      = Math.ceil(lineHeight/2);

            ctx.fontSize        = fontSize;
            ctx.fillStyle       = color;
            ctx.textBaseline    = "middle";

            var ox = 0;
            var w = 0;

            let data = this._textData;


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

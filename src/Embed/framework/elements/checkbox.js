/**
 * Copyright 2017 Nidium Inc. All rights reserved.
 * Use of this source code is governed by a MIT license
 * that can be found in the LICENSE file.
 */

{
    const Elements = require("Elements");
    const drawer = require("../core/drawer.js");
    const { ElementStyle } = require("ElementsStyles");

    Elements.checkbox = class extends Elements.Element {
        constructor(attributes={}) {
            super(attributes);

            var name = attributes.name || "";

            this.selected = attributes.selected === "selected" ? true : false;
            this.disabled = attributes.disabled === "disabled" ? true : false;

            this.style.maxWidth = 36;
            this.style.height = 36;
            this.style.flexGrow = 1;

            this.setDefaultStyle({
                cursor : "pointer",
                lineHeight : 20,
                borderWidth : 2,
                borderColor : "#3388EE",
                selectedColor : "#3388EE",
                radius : 4
            });

            this.on("click", ()=>{
                if (this.selected) {
                    this.unselect();
                } else {
                    this.select();
                }
            });
        }

        set disabled(value) {
            if (value != this._disabled) {
                this._disabled = value;
                if (value) {
                    this.cursor = "default";
                } else {
                    this.cursor = "pointer";
                }
                this.requestPaint();
            }
        }

        get disabled() {
            return this._disabled;
        }

        select() {
            if (this._disabled) return false;

            this.selected = true;
            this.value = this.attributes.value || null;
            this.requestPaint();
        }

        unselect() {
            if (this._disabled) return false;
            this.selected = false;
            this.value = null;
            this.requestPaint();
        }

        paint(ctx, w, h) {
            let s = this.style;

            var backgroundColor = s.backgroundColor;
            var selectedColor = s.selectedColor;
            var borderColor = s.borderColor;

            if (this._disabled) {
                borderColor = "#c0c0c0";
                selectedColor = "#c0c0c0";
            }

            var bounds = this.getDrawingBounds(),
                radius = h/2.5;

            if (backgroundColor) {
                ctx.fillStyle = backgroundColor;
                ctx.fillRect(0, 0, w, h, s.radius);
            }

            ctx.lineWidth = s.borderWidth;

            /* Outer Circle ------------------------------------------------- */
            
            var pad = 6.5;

            s.shadowBlur && drawer.setShadow(ctx, s);
                ctx.strokeStyle = borderColor;

                drawer.roundbox(ctx, {
                        x : bounds.x + pad,
                        y : bounds.y + pad,
                        w : bounds.h - 2*pad, // yes, h
                        h : bounds.h - 2*pad
                    },
                    s.radius-1,
                    backgroundColor, 
                    borderColor,
                    s.borderWidth
                );
            s.shadowBlur && drawer.disableShadow(ctx);

            /* Inner Background --------------------------------------------- */

            var r = 3;
            ctx.lineWidth = 0;

            if (this.selected){

                drawer.roundbox(ctx, {
                        x : bounds.x + pad + r,
                        y : bounds.y + pad + r,
                        w : bounds.h - 2*pad - 2*r, // yes, h
                        h : bounds.h - 2*pad - 2*r
                    },
                    s.radius-2,
                    selectedColor, 
                    null,
                    null
                );

            } else {
                r = 5;
                ctx.fillStyle = "rgba(0, 0, 0, 0.00)";
                ctx.strokeStyle = borderColor;

                drawer.roundbox(ctx, {
                        x : bounds.x + pad + r,
                        y : bounds.y + pad + r,
                        w : bounds.h - 2*pad - 2*r, // yes, h
                        h : bounds.h - 2*pad - 2*r
                    },
                    1,
                    null, 
                    borderColor,
                    1
                );

            }
        }
    }

    ElementStyle.Inherit(Elements.checkbox);
}
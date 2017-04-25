/**
 * Copyright 2017 Nidium Inc. All rights reserved.
 * Use of this source code is governed by a MIT license
 * that can be found in the LICENSE file.
 */

{
    const Elements = require("Elements");
    const drawer = require("../core/drawer.js");
    const { ElementStyle } = require("ElementsStyles");

    var hash = {};

    Elements.radio = class extends Elements.Element {
        constructor(attributes={}) {
            super(attributes);

            var name = attributes.name || "";

            this.selected = attributes.selected === "selected" ? true : false;

            if (name) {
                if (!hash[name]) hash[name] = [];
                hash[name].push(this);
            }

            this.cursor = "pointer";

            this.style.maxWidth = 36;
            this.style.height = 36;

            this.style.flexGrow = 1;

            this.style.lineHeight = 20;
            this.style.borderWidth = 2;
            this.style.borderColor = "#3388EE";
            this.style.selectedColor = "#3388EE";
            this.style.radius = 18;

            this.on("click", ()=>{
                var radios = this.getPeers(name);

                if (radios.length>1) {
                    for (var i=0, l=radios.length; i<l; i++) {
                        radios[i].unselect();
                    }
                    this.select();
                } else {
                    if (this.selected) {
                        this.unselect();
                    } else {
                        this.select();
                    }
                }
            });
        }

        getPeers(name) {
            return hash[name] || [];
        }

        select() {
            this.selected = true;
            this.value = this.attributes.value || null;
            this.requestPaint();
        }

        unselect() {
            this.selected = false;
            this.value = null;
            this.requestPaint();
        }

        paint(ctx, w, h) {
            let s = this.style;

            var bounds = this.getDrawingBounds(),
                radius = h/2.5;

            if (s.backgroundColor) {
                ctx.fillStyle = s.backgroundColor;
                ctx.fillRect(0, 0, w, h, s.radius);
            }

            ctx.lineWidth = s.borderWidth;

            /* Outer Circle ------------------------------------------------- */
            
            var pad = 6.5;

            s.shadowBlur && drawer.setShadow(ctx, s);
                ctx.strokeStyle = s.borderColor;

                drawer.roundbox(ctx, {
                        x : bounds.x + pad,
                        y : bounds.y + pad,
                        w : bounds.h - 2*pad, // yes, h
                        h : bounds.h - 2*pad
                    },
                    s.radius-1,
                    s.backgroundColor, 
                    s.borderColor,
                    s.borderWidth
                );
            s.shadowBlur && drawer.disableShadow(ctx);

            /* Inner Background --------------------------------------------- */

            var r = 8;
            ctx.lineWidth = 0;

            if (this.selected){
                ctx.fillStyle = s.selectedColor;
                ctx.strokeStyle = "rgba(0, 0, 0, 0.7)";

                ctx.beginPath();
                ctx.arc(
                    bounds.x+radius+(w/10),
                    bounds.y+bounds.h*0.5, 
                    radius-r, 0, 6.283185307179586, false
                );

                ctx.strokeStyle = s.borderColor;
                ctx.fill();
            } else {
                r = 8;
                ctx.fillStyle = "rgba(0, 0, 0, 0.00)";
                ctx.strokeStyle = s.borderColor;
            }

            ctx.beginPath();
            ctx.arc(
                bounds.x+radius+(w/10),
                bounds.y+bounds.h*0.5, 
                radius-r, 0, 6.283185307179586, false
            );
            ctx.fill();
            ctx.lineWidth = 1.5;
            ctx.stroke();
        }
    }

    ElementStyle.Inherit(Elements.radio);
}
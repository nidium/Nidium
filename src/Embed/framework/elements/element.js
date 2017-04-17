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
            this.onresize = this.onpaint;
        }

        onload() {
            this._ctx = this.getContext("2d");
        }

        onpaint() {
            if (!this._ctx) {
                return;
            }

            let dimensions = this.getDimensions();
            this._ctx.save();
            this.clear();
            this.paint(this._ctx, dimensions.width, dimensions.height);
            this._ctx.restore();
        }

        fadein(duration, callback) {
            var callback = callback || function(){};

            if (!this.style.opacity) this.style.opacity = 0;
            AnimationBlock(duration, Easing.Sinusoidal.Out, (style) => {
                style.opacity = 1;
            }, this.style)(callback);

            return this;
        }

        fadeout(duration, callback) {
            var callback = callback || function(){};

            if (!this.style.opacity) this.style.opacity = 1;
            AnimationBlock(duration, Easing.Sinusoidal.In, (style) => {
                style.opacity = 0;
            }, this.style)(callback);

            return this;
        }

        isVisible() {
            return this.__visible && !this.__outofbound;
        }

        ctx2d() {
            return this.getContext("2d");
        }

        getContext(type="2d") {
            return Canvas.prototype.getContext.call(this, type);
        }

        beforepaint(ctx){

        }

        paint(ctx, width, height) {

            ctx.clearRect(
                -this.coating,
                -this.coating, 
                this.clientWidth,
                this.clientHeight
            );

            ctx.clearRect(0, 0, width, height);

/*
            this.style.angle = 0;
            this.style.originOffsetX = 0;
            this.style.originOffsetY = 0;
            this.style.alpha = 1;

            var rad = this.style.angle * (Math.PI/180);

            var origin = {
                x : this.style.width/2 + this.style.originOffsetX,
                y : this.style.height/2 + this.style.originOffsetY
            };

            this.clear();

            this.beforepaint();

            ctx.save();
                ctx.globalAlpha = this.style.alpha;
                ctx.translate(origin.x, origin.y);
                ctx.rotate(rad);
                ctx.translate(-origin.x, -origin.y);
*/
                this.style._paint(ctx, width, height);
/*                
                this.afterpaint();
            ctx.restore();
*/
        }

        afterpaint(ctx){
            
        }
    }

    Elements.element = class extends Elements.Element { }
    Elements.none = Elements.Element;
}

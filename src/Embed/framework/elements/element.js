/**
 * Copyright 2017 Nidium Inc. All rights reserved.
 * Use of this source code is governed by a MIT license
 * that can be found in the LICENSE file.
 */

{

    load("embed://lib/ElementsStyles");

    Elements.Element = class extends Elements.Node {
        constructor(attributes = {}) {
            super(attributes);

            if (attributes.opacity !== undefined) {
                this.opacity = attributes.opacity;
            }

            //this.style = new ElementStyles(this); // TODO
            this.onresize = this.onpaint;
        }

        onload() {
            this._ctx = this.getContext("2d");
        }

        onpaint() {
            if (!this._ctx) {
                return;
            }

            /*
            let ctx = this._ctx;
            let { width, height } = this.getDimensions();

            this.style.angle = 0;
            this.style.originOffsetX = 0;
            this.style.originOffsetY = 0;
            this.style.alpha = 1;

            ctx.save();
                if (this.style.alpha) {
                    ctx.globalAlpha = this.style.alpha;
                } else {
                    this.clear();
                }

                if (this.style.angle) {
                    var origin = {
                        x : width*0.5 + this.style.originOffsetX,
                        y : height*0.5 + this.style.originOffsetY
                    };

                    ctx.translate(origin.x, origin.y);
                    ctx.rotate(this.style.angle * (Math.PI/180));
                    ctx.translate(-origin.x, -origin.y);
                }

                this.paint(ctx, width, height);
            ctx.restore();
            */

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
            this.style._paint(ctx, width, height);
        }

        afterpaint(ctx){
            
        }
    }

    Elements.element = class extends Elements.Element { }
    Elements.none = Elements.Element;
}

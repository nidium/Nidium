/**
 * Copyright 2017 Nidium Inc. All rights reserved.
 * Use of this source code is governed by a MIT license
 * that can be found in the LICENSE file.
 */

{
    const { StyleContainer, ElementStyle } = require("ElementsStyles");
    const { setAnimation, Easing } = require("AnimationBlock");
    const Elements = require("Elements");

    Elements.Element = class extends Elements.Node {
        constructor(attributes = {}) {
            super(attributes);

            if (attributes.opacity !== undefined) {
                this.opacity = attributes.opacity;
            }

            this.style = new (StyleContainer.Create(Object.getPrototypeOf(this)))(this);
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

            setAnimation((style) => {
                style.opacity = 1;
            }, duration, Easing.Sinusoidal.Out, this.style)(callback);

            return this;
        }

        fadeout(duration, callback) {
            var callback = callback || function(){};

            setAnimation((style) => {
                style.opacity = 0;
            }, duration, Easing.Sinusoidal.In, this.style)(callback);

            return this;
        }

        isVisible() {
            return this.__visible && !this.__outofbound;
        }

        ctx2d() {
            return this.getContext("2d");
        }

        /* This is needed because Elements.Node forbids getContext() */
        getContext(type="2d") {
            return Canvas.prototype.getContext.call(this, type);
        }
    }

    Elements.element = class extends Elements.Element { }
    Elements.none = Elements.Element;

    [
        "opacity",
        "overflow",
        "scrollLeft",
        "scrollTop",
        "allowNegativeScroll",
        "width",
        "coating",
        "height",
        "maxWidth",
        "maxHeight",
        "minWidth",
        "minHeight",
        "position",
        "display",
        "top",
        "left",
        "right",
        "bottom",
        "visible",
        "marginLeft",
        "marginRight",
        "marginTop",
        "marginBottom",
        "paddingLeft",
        "paddingRight",
        "paddingTop",
        "paddingBottom",
        "cursor",
        "flexDirection",
        "flexWrap",
        "justifyContent",
        "alignItems",
        "alignContent",
        "flexGrow",
        "flexShrink",
        "flexBasis",
        "alignSelf",
        "aspectRatio"
    ].forEach(stl => {
        ElementStyle.Implement(
            Elements.Element,
            new ElementStyle(stl, { isNative: true })
        );
    });

    ElementStyle.Inherit(Elements.element);
}

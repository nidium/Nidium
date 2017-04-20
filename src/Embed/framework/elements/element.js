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

            const classes = attributes.class;
            if (classes) {
                let nss;
                if (this.shadowRoot) {
                    // If element is a ShadowRoot, we need to get the styling
                    // information from the parent ShadowRoot
                    nss = this.getParent()[s_ShadowRoot].getNSS();
                } else {
                    nss = this.el[s_ShadowRoot].getNSS();
                }

                let tmp = [];
                for (let c of classes.split(" ")) {
                    tmp.push(nss[c]);
                }

                // Merge all style into |this.style|
                tmp.unshift(this.style);
                Object.assign.apply(null, tmp);
            }

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

        paint(ctx, width, height) {
            let bg = this.style.backgroundColor;

            if (bg) {
                ctx.fillStyle = this.style.backgroundColor;
                ctx.fillRect(0, 0, width, height);
            }
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

        getDrawingBounds() {
            var {width, height} = this.getDimensions();

            return {
                x : 0,
                y : 0,
                w : width,
                h : height
            };
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

    ElementStyle.Implement(
        Elements.Element,
        new ElementStyle("backgroundColor", { inherit: false, repaint: true })
    );

    ElementStyle.Implement(
        Elements.Element, 
        new ElementStyle("color", { inherit: true, repaint: true })
    );
    
    ElementStyle.Inherit(Elements.element);
}

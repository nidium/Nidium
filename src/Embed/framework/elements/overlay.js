/**
 * Copyright 2017 Nidium Inc. All rights reserved.
 * Use of this source code is governed by a MIT license
 * that can be found in the LICENSE file.
 */

{
    const Elements = require("Elements");
    const { StyleContainer, ElementStyle } = require("ElementsStyles");
    
    Elements.overlay = class extends Elements.Element {
        constructor(a={}) {
            super(a);

            this.style.position = "relative";
            this.style.left = 0;
            this.style.top = 0;
            this.style.width = "100%";
            this.style.height = "100%";
            this.style.backgroundColor = a.color || "black";
            this.style.opacity = 0.0;

            this.finalOpacity = a.opacity || 0.8;
        }

        show() {
            if (this.anim) this.anim.finish();
            this.style.opacity = this.finalOpacity;
            this._opening = false;
            this._closing = false;
        }

        hide() {
            if (this.anim) this.anim.finish();
            this.style.opacity = this.finalOpacity;
            this._opening = false;
            this._closing = false;
        }

        open(duration=1200) {
            if (this._opening) return false;

            if (this._closing) {
                this._closing = false;
                this.anim.cancel();
            }

            this.bringToFront();

            this._opening = true;
            this.anim = setAnimation(
                s => {
                    s.opacity = this.finalOpacity;
                },
                duration,
                Easing.Exponential.Out,
                this.style
            );

            this.anim.onFinish = () => {
                this._opening = false;
            };
        }

        close(duration=1200) {
            if (this._closing) return false;

            if (this._opening) {
                this._opening = false;
                this.anim.cancel();
            }

            this._closing = true;
            this.anim = setAnimation(
                s => {
                    s.opacity = 0;
                },
                duration,
                Easing.Exponential.Out,
                this.style
            );

            this.anim.onFinish = () => {
                this._closing = false;
                this.sendToBack();
            };
        }
    }

    ElementStyle.Inherit(Elements.overlay);
}

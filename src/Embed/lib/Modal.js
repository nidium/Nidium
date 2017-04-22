/*
 * Copyright 2017 Nidium Inc. All rights reserved.
 * Use of this source code is governed by a MIT license
 * that can be found in the LICENSE file.
 */

const Elements = require("Elements");

const { ElementStyle } = require("ElementsStyles");

class Modal extends Elements.Element {
    constructor(attributes) {
        super(attributes);

        this.style.position = "absolute";
        this.style.width = "100%";
        this.style.height = "100%";
        this.style.backgroundColor = "black";
        this.style.opacity = 0.0;

        this._opening = false;
        this._closing = false;

        this.on("mousedown", () => {
            this.close();
        });

        this.addInnerModal();
    }

    addInnerModal() {
        this.innerModal = new Elements.view();
        this.innerModal.style.position = "relative";
        this.innerModal.style.left = "5%";
        this.innerModal.style.width = "90%";
        this.innerModal.style.height = "90%";
        this.innerModal.style.backgroundColor = "#ffffff";
        this.innerModal.style.radius = 9;
        this.innerModal.style.opacity = 0;
        this.innerModal.style.top = window.innerHeight;

        document.canvas.add(this.innerModal);
    }

    open() {
        if (this._opening) return false;

        if (this._closing) {
            this._closing = false;
            this.anim.cancel();
        }

        this.bringToFront();
        this.innerModal.bringToFront();

        this._opening = true;
        this.anim = setAnimation(
            s => {
                s.opacity = 0.8;
            },
            1200,
            Easing.Exponential.Out,
            this.style
        );

        this.anim.onFinish = () => {
            this._opening = false;
            console.log("opened");
        };

        setAnimation(
            s => {
                s.top = 20;
                s.opacity = 1.0;
            },
            600,
            Easing.Exponential.Out,
            this.innerModal.style
        );
    }

    close() {
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
            1200,
            Easing.Exponential.Out,
            this.style
        );

        this.anim.onFinish = () => {
            this._closing = false;
            this.sendToBack();
            console.log("closed");
        };

        setAnimation(
            s => {
                s.top = window.innerHeight;
                s.opacity = 1.0;
            },
            600,
            Easing.Exponential.Out,
            this.innerModal.style
        );
    }
}

ElementStyle.Inherit(Modal);

module.exports = Modal;
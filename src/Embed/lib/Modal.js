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
        this.innerModal.style.height = "400";
        this.innerModal.style.backgroundColor = "#ffffff";
        this.innerModal.style.radius = 4;
        this.innerModal.style.opacity = 0;
        this.innerModal.style.flexDirection = "column";
        this.innerModal.style.justifyContent = "space-around";
        this.innerModal.style.alignItems = "flex-start";
        this.innerModal.style.top = window.innerHeight;

        document.canvas.add(this.innerModal);

        this.head = new Elements.view();
        this.head.style.position = "relative";
        this.head.style.left = "0";
        this.head.style.width = "100%";
        this.head.style.height = "55";
        this.head.style.flexDirection = "row";
        this.head.style.justifyContent = "space-between";
        this.head.style.alignItems = "center";
        this.head.style.backgroundColor = "#e0e0e0";

        this.body = new Elements.view();
        this.body.style.position = "relative";
        this.body.style.left = "0";
        this.body.style.top = "55";
        this.body.style.width = "100%";
        this.body.style.height = "345";
        this.body.style.flexDirection = "column";
        this.body.style.justifyContent = "center";
        this.body.style.alignItems = "center";

        this.innerModal.add(this.head);
        this.innerModal.add(this.body);

        var t = new Elements.element();
        t.style.flexGrow = 1;
        t.style.fontSize = 18;
        t.style.height = 30;
        t.style.lineHeight = 30;
        t.style.color = "#555555";
        t.style.marginLeft = 6;
        t.textContent = "Title";

        var b = new Elements.button();
        b.style.maxWidth = "80";
        b.style.marginRight = 6;
        b.textContent = "Close";
        b.on("click", () => {
            this.close();
        });

        this.head.add(t);
        this.head.add(b);

        this.modalTitle = t;
    }
    
    setTitle(title) {
        this.modalTitle.textContent = title;
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
            400,
            Easing.Exponential.Out,
            this.style
        );

        this.anim.onFinish = () => {
            this._closing = false;
            this.sendToBack();
        };

        setAnimation(
            s => {
                s.top = window.innerHeight;
                s.opacity = 1.0;
            },
            700,
            Easing.Exponential.Out,
            this.innerModal.style
        );
    }
}

Modal.spinner = function(state) {
    if (state) {
        if (!this.__overlay) {
            var overlay = new Elements.overlay({
                opacity : 0.50
            });

            overlay.style.justifyContent = "center";
            overlay.style.alignItems = "center";

            var spinner = new Elements.spinner({
                color:'#ffffff',
                width:30,
                height:30,
                radius:8,
                dashes:20,
                opacity:0.7,
                speed:50
            });

            overlay.add(spinner);
            document.canvas.add(overlay);
            this.__overlay = overlay;
        }

        this.__overlay.open(200, Easing.Sinusoidal.Out);
    } else {
        if (!this.__overlay) return false;
        this.__overlay.close(800, Easing.Sinusoidal.Out);
    }
};

ElementStyle.Inherit(Modal);

module.exports = Modal;
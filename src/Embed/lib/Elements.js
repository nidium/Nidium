/*
   Copyright 2016 Nidium Inc. All rights reserved.
   Use of this source code is governed by a MIT license
   that can be found in the LICENSE file.
*/

const cssParse = require("css-parse");

var Elements = {};

module.exports = Elements;

class NidiumNode extends Canvas {
    constructor(attributes = {}) {
        super(attributes.width || 10, attributes.height || 10);

        this.attributes = attributes;
        this.computedAttributes = {};

        this.left = attributes.left || 0;
        this.top = attributes.top || 0;
        this.opacity = attributes.opacity || 1;

        this.onload = function() {
            this._ctx = this.getContext("2d");
        }

        this.onpaint = function() {
            this._ctx.save();
            this.clear();
            this.paint(this._ctx);
            this._ctx.restore();
        }

        //this.onload = this.onpaint;
        this.onresize = this.onpaint;

    }

    setAttribute(attr, value) {
        switch(attr) {
            case 'height':
            console.log("Height set");
            this.height = parseInt(value);
            break;
        }

        this.computedAttributes[attr] = value;
        this.requestPaint();
    }

    name() {
        return "DefaultNode"
    }

    ctx2d() {
        return this.getContext("2d");
    }

    paint(ctx) {}
}


Elements.UIButton = class extends NidiumNode {
    constructor(attributes) {
        super(attributes);

        this.cursor = "pointer";

        this.on("mouseup", function(ev) {
            AnimationBlock(500, Easing.Back.Out, function(btn) {
                btn.width += 20;
                btn.height += 20;
                /* TODO: stopPropagation doesn't work? */
                ev.stopPropagation();


            }, this);
        });
    }

    set label(value) {
        this._label = value;
        this.requestPaint();
    }

    get label() {
        return this._label;
    }

    name() {
        return "UiButton";
    }

    paint(ctx) {
        ctx.fillStyle = "#aaa";
        ctx.stokeStyke = "#111";

        ctx.fillRect(0, 0, this.width, this.height, 15, 15);
        ctx.strokeRect(0, 0, this.width-0.5, this.height-0.5, 15, 15);

        ctx.fillStyle = "#000";
        ctx.textAlign = "center";

        ctx.fillText(this._label, this.width/2, this.height/2+4);
    }
}

Elements.section = class extends NidiumNode {
    constructor(attributes) {
        super(attributes);

        var mr = (min=100, max=200) => min + Math.floor(Math.random()*(max-min));
        this._color = `rgba(${mr(70, 100)}, ${mr(120, 200)}, ${mr(140, 210)}, 0.8)`;
    }

    name() {
        return "section";
    }

    paint(ctx) {
        ctx.fillStyle = this._color;
        ctx.fillRect(0, 0, this.width, this.height);
        ctx.strokeStyle = "rgb(0, 255, 255)";
        ctx.strokeRect(0.5, 0.5, this.width-1, this.height-1);

        if (this.computedAttributes.label) {
            ctx.fillStyle = "#000";
            ctx.textAlign = "center";
            ctx.fontSize = 20;
            ctx.fillText(this.computedAttributes.label, this.width / 2, this.height - 20);
        }
    }
}

Elements.div = Elements.section;

Elements.img = class extends NidiumNode {
    constructor(attributes) {
        super(attributes);
        this.src = attributes.src;
        this._loaded = false;
    }

    set src(value) {
        this._src = value;
        this._img = new Image();
        this._img.src = value;

        this._img.onload = () => {
            this._loaded = true;
            this.setSize(this._img.width, this._img.height);
            this.requestPaint();
        }
    }

    get src() {
        return this._src;
    }

    paint(ctx) {
        if (this._loaded) {
            ctx.drawImage(this._img, 0, 0);
        }
    }

}

window._onready = function(lst) {
    document.canvas.inject(lst);
}

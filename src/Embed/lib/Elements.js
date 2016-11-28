/*
   Copyright 2016 Nidium Inc. All rights reserved.
   Use of this source code is governed by a MIT license
   that can be found in the LICENSE file.
*/

const Elements = {

    Create(tag, attributes) {
        if (!(tag in Elements)) {
            throw Error(`${tag} tag type is not implemented`);
            return;
        }

        return new Elements[tag](attributes);
    },

    Exists(tag) {
        return (tag in Elements);
    }

};

class NidiumNode extends Canvas {
    constructor(attributes = {}) {
        super(attributes.width || 10, attributes.height || 10);

        this.attributes = attributes;
        this.computedAttributes = {};

        this.left = attributes.left || 0;
        this.top = attributes.top || 0;
        this.opacity = attributes.opacity || 1;

        //this.onload = this.onpaint;
        this.onresize = this.onpaint;
        this._textValue = "";
    }

    onload() {
        this._ctx = this.getContext("2d");
    }

    onpaint() {
        if (!this._ctx) return;

        this._ctx.save();
        this.clear();
        this.paint(this._ctx);
        this._ctx.restore();
    }

    removeChild(child) {
        if (!child) {
            return;
        }

        var parent = child.getParent();
        if (!parent) {
            return;
        }

        if (parent.idx != this.idx) {
            return;
        }

        child.removeFromParent();
    }

    hasAttribute(attr) {
        return (attr in this.attributes);
    }

    set innerHTML(value) {
        console.log("Setting innver html", this.idx, value);
        this._innerhtml = value;
        this.replaceWith(value);
    }

    set textContent(value) {
        /* Don't create a node if there is already a textNode as an only child */
        var c = this.getChildren();
        if (c.length == 1 && c[0].nodeType == 3) {
            c[0].nodeValue = value;
            return;
        }

        this.empty();
        this.add(new Elements.textNode(value));
    }

    get textContent() {
        return this._textValue;
    }

    get text() {
        throw Error("text property doesnt exist");
    }

    get innerHTML() {
        return this._innerhtml;
    }

    get firstChild() {
        console.log("Firstchild...");
    }

    get tagName() {
        return this.name();
    }

    get nodeType() {
        return 1;
    }

    get parentNode() {
        return this.getParent();
    }

    get nextSibling() {
        return this.getNextSibling();
    }

    get childNodes() {
        return this.getChildren();
    }

    get textContent() {
        return this._textValue || "";
    }

    cloneNode(deep = true) {
        var clone = new this.constructor(this.attributes);

        if (!deep) {
            return clone;
        }

        for (let child of this.getChildren()) {
            clone.add(child.clone(true));
        }

        return clone;
    }

    setAttribute(attr, value) {
        console.log("Set attr", attr, this.idx, value);

        switch(attr) {
            case 'height':
                this.height = parseInt(value);
                break;
            default:
                this[attr] = value;
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

Elements.textNode = class extends NidiumNode {

    constructor(textValue) {
        super(1, 1);
        this._textValue = textValue;
    }

    /* We don't want a textNode to create a gfx context */
    onload() {}
    onpaint() {}
    ctx2d() { return null; }
    getContext() { return null; }

    setParentText() {
        var parent = this.getParent();
        if (!parent) {
            return;
        }

        parent._textValue = this._textValue;
        parent.fireEvent("textchanged", this._textValue);
        parent.requestPaint();
    }

    add() {
        throw Error("textNode doesn't support this operation");
    }
    addSubCanvas() {
        throw Error("textNode doesn't support this operation");
    }
    appendChild() {
        throw Error("textNode doesn't support this operation");
    }

    onmount() {
        this.setParentText();
    }

    get nodeValue() {
        return this._textValue;
    }

    set nodeValue(textValue) {
        this._textValue = textValue;
        this.setParentText();
        this.fireEvent("nodeValueChanged", textValue);
    }

    name() {
        return "textNode";
    }

    get nodeType() {
        return 3;
    }
}

Elements.Canvas = class extends NidiumNode {

}

Elements.UIButton = class extends NidiumNode {
    constructor(attributes) {
        super(attributes);

        this.cursor = "pointer";
        this.position = "inline";

        this.on("mouseup", function(ev) {
            AnimationBlock(500, Easing.Back.Out, function(btn) {
                btn.width += 20;
                btn.height += 20;
                /* TODO: stopPropagation doesn't work? */
                ev.stopPropagation();


            }, this);
        });
    }

    name() {
        return "UiButton";
    }

    ontextchanged(newtext) {
        var ctx = this.ctx2d();

        var data = ctx.measureText(newtext);

        this.width = data.width + 30;
    }

    paint(ctx) {
        ctx.fillStyle = "#aaa";
        ctx.stokeStyke = "#111";

        ctx.fillRect(0, 0, this.width, this.height, 15, 15);
        ctx.strokeRect(0, 0, this.width-0.5, this.height-0.5, 15, 15);

        ctx.fillStyle = "#000";
        ctx.textAlign = "center";

        ctx.fillText(this._textValue, this.width/2, this.height/2+4);
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

Elements.div = class extends Elements.section {
    constructor(attributes) {
        super(attributes);
        this.position = "inline";
        this.staticRight = true;
        this.right = 0;
    }

    name() {
        return "div";
    }

    onmount() {
        this.width = this.getParent().width;
    }
}

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

module.exports = Elements;

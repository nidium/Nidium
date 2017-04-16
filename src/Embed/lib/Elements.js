/*
 * Copyright 2017 Nidium Inc. All rights reserved.
 * Use of this source code is governed by a MIT license
 * that can be found in the LICENSE file.
 */

const Elements = module.exports = {};

const ShadowRoot    = require("../ShadowRoot.js");
const g_MainShadow  = new ShadowRoot(document.canvas, {"name": "main"});

Object.defineProperty(document.canvas, "shadowRoot", {
    "writable": false,
    "configurable": false,
    "value": g_MainShadow
});

Elements.currentShadow = null;

Elements.NodeType = {
    ELEMENT_NODE : 1,
    TEXT_NODE : 3,
    PROCESSING_INSTRUCTION_NODE : 7,
    COMMENT_NODE : 8,
    DOCUMENT_NODE : 9,
    DOCUMENT_TYPE_NODE : 10,
    DOCUMENT_FRAGMENT_NODE : 11
};

<<<<<<< HEAD
Elements.Create = function(tag, attributes, shadowRoot=g_MainShadow) {
    tag = tag.toLowerCase();
    let ret;

    let previousShadow = Elements.currentShadow;
    Elements.currentShadow = shadowRoot;

    try {
        if (!(tag in Elements)) {
            throw Error(`<${tag}> is not implemented.`);
=======
class NidiumNode extends Canvas {
    constructor(attributes = {}) {
        super(attributes.width, attributes.height);

        this.attributes = attributes;
        this.computedAttributes = {};

        if (attributes.opacity !== undefined) {
            this.opacity = attributes.opacity;
        }

        //this.onload = this.onpaint;
        this.onresize = this.onpaint;
        this._textValue = "";
    }

    getNMLContent(self = true) {
        var childContent = ''

        for (let child of this.getChildren()) {
            childContent += child.getNMLContent(true);
        }

        if (!self) {
            return childContent;
        }

        let tag = this.name();
        var attributestr = '';

        for (let prop of Object.getOwnPropertyNames(this.attributes)) {
            attributestr += ` ${prop}="${this.attributes[prop]}"`;
        }

        return `<${tag}${attributestr}>${childContent}</${tag}>`
    }

    allowsChild() {
        return true;
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
        this.replaceWith(value);
    }

    get innerHTML() {
        return this.getNMLContent(false);
    }

    set textContent(value) {
        /* Don't create a node if there is already a textNode as an only child */
        var c = this.getChildren();
        if (c.length == 1 && c[0].nodeType == 3) {
            c[0].nodeValue = value;
            return;
        }

        this.empty();
        this.add(Elements.Create("textnode", value));
    }

    get textContent() {
        return this._textValue;
    }

    get text() {
        throw Error("text property doesnt exist");
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
            clone.add(child.cloneNode(true));
        }

        return clone;
    }

    setAttribute(attr, value) {
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

Elements.textnode = class extends NidiumNode {

    constructor(textValue) {
        super(1, 1);
        this._textValue = textValue;
    }

    cloneNode(deep = true) {
        return new this.constructor(this._textValue);
    }

    /* We don't want a textNode to create a gfx context */
    onload() {}
    onpaint() {}
    ctx2d() { return null; }
    getContext() { return null; }

    setParentText() {
        var parent = this.getParent();
        if (!parent) {
>>>>>>> 733b6060f0b1b6d75fc82141f42d926eb554849c
            return;
        }

        if (!Elements[tag].__NodeName__) Elements[tag].__NodeName__ = tag;

        ret = new Elements[tag](attributes);
    } finally {
        Elements.currentShadow = previousShadow;
    }

<<<<<<< HEAD
    return ret;
=======
    paint(ctx, width, height) {
        ctx.fillStyle = this._color;
        ctx.fillRect(0, 0, width, height);
        ctx.strokeStyle = "rgb(0, 255, 255)";
        ctx.strokeRect(0.5, 0.5, width-1, height-1);

        if (this.computedAttributes.label) {
            ctx.fillStyle = "#000";
            ctx.textAlign = "center";
            ctx.fontSize = 20;
            ctx.fillText(this.computedAttributes.label, width / 2, height - 20);
        }
    }
>>>>>>> 733b6060f0b1b6d75fc82141f42d926eb554849c
}

Elements.Exists = function(tag) {
    return (tag.toLowerCase() in Elements);
}

/*
    Generic resource loader : return the text content
    of the file pointed by src attribute
*/
Elements.Loader = function(attributes) {
    let data;

    if (attributes && attributes.src) {
        let src = attributes.src;
        try {
            data = File.readSync(src, {"encoding": "utf-8"});
        } catch (e) {
            console.error(`Failed to load ${src}: ${e}`);
            return "";
        }

        if (!data) {
            console.warn(`${src} is empty.`);
            return "";
        }
    }
}

Elements.flexcanvas = class extends NidiumNode {
    constructor(attributes) {
        super(attributes);

        this._color = attributes.color || "red";
    }

<<<<<<< HEAD
    return data;
=======
    paint(ctx, width, height) {
        ctx.fillStyle = this._color;
        ctx.fillRect(0, 0, width, height);
        ctx.fillStyle = "black";
        ctx.fillText(this.idx, 0, 10);
    }
>>>>>>> 733b6060f0b1b6d75fc82141f42d926eb554849c
}


window._onready = function(lst){};

module.exports = Elements;

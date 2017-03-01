/*
   Copyright 2016 Nidium Inc. All rights reserved.
   Use of this source code is governed by a MIT license
   that can be found in the LICENSE file.
*/

const Elements = module.exports = {};

const ElementStyles = require("./Styles.js");
const ShadowRoot    = require("../../ShadowRoot.js");
const s_ShadowRoot  = require("../../Symbols.js").ElementShadowRoot;
const s_NodeName    = Symbol("NodeName");
const s_NodeID      = Symbol("NodeID");

const g_MainShadow  = new ShadowRoot(document.canvas, {"name": "main"});
let g_CurrentShadow = null;

Elements.Create = function(tag, attributes, shadowRoot=g_MainShadow) {
    tag = tag.toLowerCase();
    let ret;

    let previousShadow = g_CurrentShadow;
    g_CurrentShadow = shadowRoot;

    try {
        if (!(tag in Elements)) {
            throw Error(`Tag <${tag}> is not implemented`);
            return;
        }

        if (!Elements[tag][s_NodeName]) Elements[tag][s_NodeName] = tag;

        ret = new Elements[tag](attributes);
    } finally {
        g_CurrentShadow = previousShadow;
    }

    return ret;
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
            console.error(`Failed to load ${src} : ${e}`);
            return "";
        }

        if (!data) {
            console.warn(`No data for ${src}`);
            return "";
        }
    }

    return data;
}

/*
    Node are container only canvas (no GFX context)
*/
const Node = Elements.Node = class extends Canvas {
    constructor(attributes = {}) {
        super(attributes.width || 10, attributes.height || 10);

        this.attributes = attributes;
        this.computedAttributes = {};

        this.left = attributes.left || 0;
        this.top = attributes.top || 0;

        this[s_ShadowRoot] = g_CurrentShadow;
        this[s_ShadowRoot].addTag(this.name(), this);

        this.addEventListener("load", () => {
            this.fireEvent("mount", {});
        });
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

        return `<${tag}${attributestr}>${childContent}</${tag}>`;
    }

    allowsChild() {
        return true;
    }

    getParent() {
        let p = super.getParent();
        if (!p || p[s_ShadowRoot] != this[s_ShadowRoot]) {
            return null;
        } else {
            return p;
        }
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

    removeFromParent() {
        this[s_ShadowRoot].rm(this);
        super.removeFromParent();
    }

    add(child) {
        if (!this.allowsChild()) {
            throw Error(`<${this.name()}> does not support children`);
        }
        this[s_ShadowRoot].add(child);
        return super.add(child);
    }

    addSubCanvas(child) {
        return this.add(child);
    }

    appendChild(child) {
        return this.add(child);
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
        if (c.length == 1 && c[0].nodeType == Node.TEXT_NODE) {
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
        return Node.ELEMENT_NODE;
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

    cloneNode(deep = true, shadowRoot=this[s_ShadowRoot]) {
        var clone = Elements.Create(this.name(), this.attributes, shadowRoot);

        if (!deep) {
            return clone;
        }

        for (let child of this.getChildren()) {
            clone.add(child.cloneNode(true, shadowRoot));
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
        return this.constructor[s_NodeName];
    }

    attachShadow(options) {
        options = options || {};
        let ret = new ShadowRoot(this, options);
        this[s_ShadowRoot] = ret;
        // TODO : Move children outside of shadowroot (spec)
        return ret;
    }

    get shadowRoot() {
        let shadow = this[s_ShadowRoot];
        return shadow && shadow.host == this ? shadow : null;
    }

    getRootNode(options={}) {
        let p = Canvas.prototype.getParent.call(this);
        while (p) {
            if (!options.composed && p.shadowRoot) {
                return p;
            }
            p = Canvas.prototype.getParent.call(p);
        }
        return p;
    }

    get id() {
        return this[s_NodeID];
    }

    set id(val) {
        this[s_NodeID] = val;
        let shadow = this[s_ShadowRoot]

        shadow.addID(val, this);
    }

    createTree(children) {
        if (this.allowsChild()) {
            this.addMultiple(...NML.CreateTree(children, this[s_ShadowRoot]));
        }
    }

    onload() {}
    onpaint() {}
    ctx2d() { return null; }
    getContext() { return null; }
}

Node.ELEMENT_NODE = 1;
Node.TEXT_NODE = 3;
Node.PROCESSING_INSTRUCTION_NODE = 7;
Node.COMMENT_NODE = 8;
Node.DOCUMENT_NODE = 9;
Node.DOCUMENT_TYPE_NODE = 10;
Node.DOCUMENT_FRAGMENT_NODE = 11;

Elements.Element = class extends Elements.Node {
    constructor(attributes) {
        super(attributes);

        if (attributes.opacity !== undefined) {
            this.opacity = attributes.opacity;
        }

        this.style = new ElementStyles(this);

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

    ctx2d() {
        return this.getContext("2d");
    }

    getContext(type="2d") {
        return Canvas.prototype.getContext.call(this, type);
    }

    paint(ctx) {
        this.style.paint(ctx);
    }
}

const s_NodeText = Symbol("NodeText");
Elements.textnode = class extends Elements.Element {

    constructor(textValue) {
        super(1, 1);
        this.nodeValue = textValue;
    }

    getNMLContent() {
        return this.nodeValue;
    }

    set textContent(value) {
        this.nodeValue = value;
    }

    allowsChild() {
        return false;
    }

    get nodeValue() {
        return this[s_NodeText];
    }

    set nodeValue(textValue) {
        this[s_NodeText] = textValue.trim();
        this.requestPaint();
        this.fireEvent("nodeValueChanged", textValue);
    }

    get nodeType() {
        return Node.TEXT_NODE;
    }

    paint(ctx) {
        let maxWidth    = this.getParent().width;
        let data        = ctx.breakText(this.nodeValue, maxWidth);
        let actualWidth = 1;

        // FIXME : Get these values from inherited styles
        let fontSize    = 15;
        let lineHeight  = 20;
        let color       = "black";
        let offset      = Math.ceil(lineHeight/2);

        ctx.fontSize        = fontSize;
        ctx.fillStyle       = color;
        ctx.textBaseline    = "middle";

        for (var i = 0; i < data.lines.length; i++) {
            let tmp = ctx.measureText(data.lines[i])
            if (tmp.width > actualWidth) actualWidth = tmp.width;

            ctx.fillText(data.lines[i], 0, i * lineHeight + offset);
        }

        this.width  = actualWidth;
        this.height = lineHeight * data.lines.length;
    }
}

Elements.element = class extends Elements.Element { }

Elements.canvas = class extends Elements.Element {
    /*
        regular <canvas> are "low level"
        Don't clear the buffer
    */
    onpaint() {}
}

Elements.uibutton = class extends Elements.Element {
    constructor(attributes) {
        super(attributes);

        this.cursor = "pointer";
        this.position = "inline";

        this.on("mouseup", function(ev) {
            AnimationBlock(500, Easing.Back.Out, function(btn) {

                /* TODO: stopPropagation doesn't work? */
                ev.stopPropagation();


            }, this);
        });
    }

    paint(ctx) {
        ctx.fillStyle = "#aaa";
        ctx.stokeStyke = "#111";

        ctx.fillRect(0, 0, this.width, this.height, 15, 15);
        ctx.strokeRect(0, 0, this.width-0.5, this.height-0.5, 15, 15);
    }
}

Elements.section = class extends Elements.Element {
    constructor(attributes) {
        super(attributes);

        var mr = (min=100, max=200) => min + Math.floor(Math.random()*(max-min));
        this._color = `rgba(${mr(70, 100)}, ${mr(120, 200)}, ${mr(140, 210)}, 0.8)`;
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

Elements.none = Elements.Element;

Elements.div = class extends Elements.Element {
    constructor(attributes) {
        super(attributes);
        this.position = "inline";
        this.staticRight = true;
        this.right = 0;
    }

    paint(ctx) {
        super.paint(ctx)
        ctx.fillStyle = "#000";
        ctx.fillText(this._textValue, 0, this.height/2+4);
    }

    onmount() {
        this.width = this.getParent().width;
    }
}

Elements.img = class extends Elements.Element {
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

            if (this.attributes.onload) {
                this.attributes.onload.call(this, this._img);
            }

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

load("embed://lib/Elements/layout.js");
load("embed://lib/Elements/nss.js");
load("embed://lib/Elements/script.js");
load("embed://lib/Elements/template.js");
load("embed://lib/Elements/component.js");

window._onready = function(lst) {}

module.exports = Elements;

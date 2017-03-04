/*
   Copyright 2017 Nidium Inc. All rights reserved.
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

Object.defineProperty(document.canvas, "shadowRoot", {
    "writable": false,
    "configurable": false,
    "value": g_MainShadow
});

let g_CurrentShadow = null;

Elements.Create = function(tag, attributes, shadowRoot=g_MainShadow) {
    tag = tag.toLowerCase();
    let ret;

    let previousShadow = g_CurrentShadow;
    g_CurrentShadow = shadowRoot;

    try {
        if (!(tag in Elements)) {
            throw Error(`<${tag}> is not implemented.`);
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
            console.error(`Failed to load ${src}: ${e}`);
            return "";
        }

        if (!data) {
            console.warn(`${src} is empty.`);
            return "";
        }
    }

    return data;
}

/*
    Node are container only canvas (no GFX context)
*/
const s_FnFixStaticRight = Symbol("NodeFunctionFixStaticRight");

const Node = Elements.Node = class extends Canvas {
    constructor(attributes = {}) {
        super(parseInt(attributes.width) || 10, parseInt(attributes.height) || 10);

        this.attributes         = attributes;
        this.computedAttributes = {};

        this.position = "inline";
        this.left     = attributes.left || 0;
        this.top      = attributes.top || 0;

        // By default, node take the full width
        if (!attributes.width) {
            // XXX : Using staticRight + fluidHeight cause the canvas to only
            // be displayed on 1px : https://github.com/nidium/Nidium/issues/57
            /*
            this.staticRight = true;
            this.right       = 0;
            */
            this._fixStaticRight = true;

            this.addEventListener("load", () => {
                this[s_FnFixStaticRight]();
                var p = Canvas.prototype.getParent.call(this);
                p.addEventListener("resize", this[s_FnFixStaticRight].bind(this));
            });
        }

        // And adjust their height to the content
        this.height = attributes.height ? attributes.height : "auto";
        this.width = attributes.width ? attributes.width : "auto";

        this[s_ShadowRoot] = g_CurrentShadow;
        this[s_ShadowRoot].addTag(this.name(), this);

        if (attributes.id) {
            this.id = attributes.id;
        }

        this.addEventListener("load", () => {
            console.log("mounted", this.name(), this.width);
            this.fireEvent("mount", {});
        });
    }

    [s_FnFixStaticRight]() {
        var parent = Canvas.prototype.getParent.call(this);
        if (!this._fixStaticRight) return;

        super.width = parent.width;
    }

    // Setting width or height, must disable fluidWidth and
    // staticRight to allow the sizing of the element.
    // XXX : Should this be the default for canvas ?
    // {{{ width & height getter/setter
    set width(val) {
        if (val == "auto") {
            val = 1;
            this.fluidWidth = true;
        } else {
            this.fluidWidth = false;
        }
        //this.staticRight = false;
        this._fixStaticRight = false;
        super.width = val;
    }

    get width() {
        return super.width;
    }

    set height(val) {
        if (val == "auto") {
            this.fluidHeight = true;
            val = 1;
        } else {
            this.fluidHeight = false;
        }
        super.height = val;
    }

    get height() {
        return super.height;
    }

    set minWidth(val) {
        //this.staticRight = false;
        this._fixStaticRight = false;
        super.minWidth      = val;
    }

    get minWidth() {
        return super.minWidth;
    }

    set minHeight(val) {
        this.fluidHeight = false;
        super.minHeight  = val;
    }

    get minHeight() {
        return super.minHeight;
    }

    set maxWidth(val) {
        this._fixStaticRight = false;
        super.maxWidth      = val;
    }

    get maxWidth() {
        return super.maxWidth;
    }

    set maxHeight(val) {
        this.fluidHeight = false;
        super.maxHeight  = val;
    }

    get maxHeight() {
        return super.maxHeight;
    }
    // }}}

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

        if (!parent || parent.idx != this.idx) {
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
            throw Error(`<${this.name()}> can't have children.`);
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
        this.add(
            Elements.Create("textnode", value)
        );
    }

    get textContent() {
        return this._textValue;
    }

    get text() {
        throw Error("text property doesnt exist");
    }

    get firstChild() {
        return this.getChildren()[0] || null;
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
        switch (attr) {
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
            NML.CreateTree(children, this, this[s_ShadowRoot]);
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

Elements.element = class extends Elements.Element { }
Elements.none = Elements.Element;

/**
 * Node extends
 */
load("embed://lib/Elements/layout.js");
load("embed://lib/Elements/nss.js");
load("embed://lib/Elements/script.js");
load("embed://lib/Elements/template.js");
load("embed://lib/Elements/component.js");
load("embed://lib/Elements/slot.js");

/*
 * Native UI Controls and NML tags
 */
load("embed://lib/Elements/controls/button.js");
load("embed://lib/Elements/controls/canvas.js");
load("embed://lib/Elements/controls/div.js");
load("embed://lib/Elements/controls/img.js");
load("embed://lib/Elements/controls/section.js");
load("embed://lib/Elements/controls/textnode.js");

window._onready = function(lst){};

module.exports = Elements;

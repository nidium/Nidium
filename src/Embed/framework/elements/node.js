/**
 * Copyright 2017 Nidium Inc. All rights reserved.
 * Use of this source code is governed by a MIT license
 * that can be found in the LICENSE file.
 */

{
    const Elements      = require("Elements");
    const ShadowRoot    = require("../../ShadowRoot.js");
    const s_ShadowRoot  = require("../../Symbols.js").ElementShadowRoot;
    const s_ShadowHost  = require("../../Symbols.js").ElementShadowHost;
    const VM            = require("VM");

    Elements.Node = class extends Canvas {
        constructor(attributes = {}) {
            super(attributes.width, attributes.height);

            const shadowRoot = Elements.currentShadow || document.canvas.shadowRoot;

            for (let k of Object.getOwnPropertyNames(attributes)) {
                this.parseComposedAttribute(k, attributes, shadowRoot);
            }

            this.attributes         = attributes;
            this.computedAttributes = {};

            this[s_ShadowRoot] = shadowRoot;
            this[s_ShadowRoot].addTag(this.name(), this);

            if (attributes.id) {
                this.id = attributes.id;
            }

            this.addEventListener("load", () => {
                this.fireEvent("mount", {});
            });
        }

        /*
           parse selector:attr attributes (i.e: js:data='foobar')
        */
        parseComposedAttribute(k, attributes, shadowRoot) {
            const options = {};
            const parts = k.split(':');
            const selector = parts[0];

            if (parts.length < 2) return false;

            const attr = parts[1];

            const isJS = selector == 'js';
            const isOn = selector == 'on';

            var value = attributes[k];

            if (isJS || isOn) {

                if (shadowRoot && shadowRoot.jsScope) {
                    /*
                    options.scope = shadowRoot.jsScope;
                    options.scope.__this = shadowRoot.jsScope["this"];
                    */
                    options.scope = shadowRoot.jsScope["this"];
                }

                if (isOn) {
                    value = `function(){${value}}.bind(this)`;
                } else {
                    //value = `function() { return ${value} }.apply(__this)`
                }

                value = "(" + value + ")";

                const result = VM.run(value, options);

                if (isOn) {
                    this["on" + attr] = result;
                } else {
                    attributes[attr] = result;
                }
            }
        }

        getNMLContent(self = true) {
            var childContent = '';

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

            if (this.shadowRoot) this.shadowRoot.add(child);
            else if (this[s_ShadowRoot]) this[s_ShadowRoot].add(child);

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
            
            if (c.length == 1 && c[0].nodeType == Elements.NodeType.TEXT_NODE) {
                c[0].nodeValue = value;
                return;
            }

            this.empty();
            this.add(
                Elements.Create("textnode", value)
            );
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
                default:
                    this[attr] = value;
                    break;
            }

            this.computedAttributes[attr] = value;
        }

        name() {
            return this.constructor.__NodeName__;
        }

        attachShadow(options) {
            options = options || {};
            let ret = new ShadowRoot(this, options);
            // TODO : Move children outside of shadowroot (spec)
            return ret;
        }

        getRootNode(options={}) {
            let p = this.getParent();
            
            while (p) {
                if (!options.composed && p.shadowRoot) {
                    return p;
                }
                p = p.getParent();
            }

            return p;
        }

        get id() {
            return this.__NodeID__;
        }

        set id(val) {
            this.__NodeID__ = val;
            let shadow = this[s_ShadowRoot];

            super.id = val;

            shadow.addID(val, this);
        }

        getElementById(id) {
            return this.shadowRoot.getElementById(id);
        }

        createTree(children) {
            if (this.allowsChild()) {
                NML.CreateTree(children, this, this.shadowRoot ? this.shadowRoot : this[s_ShadowRoot]);
            }
        }

        onload() {}
        onpaint() {}
        ctx2d() { return null; }
        getContext() { return null; }

        /* ----------------------------------------------- */
        /* READ ONLY GETTERS                               */
        /* ----------------------------------------------- */

        get shadowRoot() {
            return this[s_ShadowHost];
        }

        get text() {
            throw Error("text property doesnt exist");
        }

        get firstChild() {
            return this.getFirstChild();
        }

        get lastChild() {
            return this.getLastChild();
        }

        get tagName() {
            return this.name();
        }

        get nodeType() {
            return Elements.NodeType.ELEMENT_NODE;
        }

        get parentNode() {
            return this.getParent();
        }

        get prevSibling() {
            return this.getPrevSibling();
        }

        get nextSibling() {
            return this.getNextSibling();
        }

        get childNodes() {
            return this.getChildren();
        }

    }


    /* -- click, dblclick, mousehold -- */

    const pointerHoldTreshold = 1200;
    const doubleClickTreshold = 250;

    const timer = function(fn, ms, loop, execFirst){
        var t = {
            loop : loop,
            tid : loop ? setInterval(function(){fn.call(t);}, ms)
                        : setTimeout(function(){fn.call(t);}, ms),

            remove : function(){
                if (this.loop) {
                    clearInterval(this.tid);
                } else {
                    clearTimeout(this.tid);
                }
                delete(this.tid);
            }
        };

        if (execFirst) {
            fn.call(t);
        }
        
        return t;
    };

    const distance = function(x1, y1, x2, y2){
        var a = y2-y1, b = x2-x1;
        return Math.sqrt(a*a + b*b);
    };

    Elements.Node.prototype.onmousedown = function(e){
        var self = this,
            o = this.__lastmouseevent__ || {x:0, y:0},
            dist = distance(o.x, o.y, e.x, e.y) || 0;

        e.time = +new Date();
        e.duration = null;

        window.mouseX = e.x;
        window.mouseY = e.y;

        if (e.which == 3) {
            this.fireEvent("contextmenu", e);
        }
        this.__mousedown__ = true;

        this.__mousetimer__ = timer(function(){
            self.fireEvent("holdstart", e);
            self.__mousehold__ = true;
            this.remove();
        }, pointerHoldTreshold);

        if (this.__lastmouseevent__){
            e.duration = e.time - this.__lastmouseevent__.time;

            if (dist<4 && e.duration <= doubleClickTreshold) {
                this.fireEvent("dblclick", e);
            }
        }

        this.__lastmouseevent__ = e;
    };

    Elements.Node.prototype.onmousemove = function(e){
        window.mouseX = e.x;
        window.mouseY = e.y;

        if (this.__mousedown__ && this.__mousetimer__){
            this.__mousetimer__.remove();
        }
    };

    Elements.Node.prototype.onmouseup = function(e){
        var o = this.__lastmouseevent__ || {x:0, y:0},
            dist = distance(o.x, o.y, e.x, e.y) || 0;

        // mouseup can be fired without a previous mousedown
        // this prevent mouseup to be fired after (MouseDown + Refresh)

        if (this.__mousedown__ === false) {
            return false;
        }

        this.__mousedown__ = false;

        if (this.__mousetimer__) {
            this.__mousetimer__.remove();
        }

        if (this.__mousehold__ && dist<10) {
            this.fireEvent("holdend", e);
            this.__mousehold__ = false;
            return;
        }
        if (o && dist<10) {
            //console.log(this.name(), "mouseclick")
            this.fireEvent("click", e);
        }
    };

}
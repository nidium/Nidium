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
            super();

            const shadowRoot = Elements.currentShadow || document.canvas.shadowRoot;

            for (let k of Object.getOwnPropertyNames(attributes)) {
                this.parseComposedAttribute(k, attributes, shadowRoot);
            }

            this.attributes         = attributes;
            this.computedAttributes = {};

            this.left     = attributes.left || 0;
            this.top      = attributes.top || 0;
            this.coating  = attributes.coating || 20;

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

            if (parts.length<2) return false;

            const attr = parts[1];

            const isJS = selector == 'js';
            const isOn = selector == 'on';

            var value = attributes[k];

            if (isJS || isOn) {

                if (shadowRoot && shadowRoot.jsScope) {
                    options.scope = shadowRoot.jsScope["this"];
                }

                if (isOn) {
                    value = "function(){"+value+"}.bind(this)";
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

        shader(url, callback){
            var self = this;

            File.read(url, {encoding: "utf8"}, function(err, source){
                var uniforms = {},
                    ctx = self.getContext("2d"),
                    program = ctx.attachFragmentShader(source);

                var setUniformValue = function(location, value){
                    program.uniform1i(location, value);
                };

                var createUniformBinding = function(uniform){
                    var name = uniform.name,
                        location = uniform.location,
                        type = uniform.type;

                    Object.defineProperty(uniforms, name, {
                        configurable : false,
                        get : function(){
                            return uniform.value ? uniform.value : 0;
                        },

                        set : function(value){
                            uniform.value = value;
                            setUniformValue(location, value);
                        }
                    });
                };

                var uniformArray = program.getActiveUniforms();

                for (var i=0; i<uniformArray.length; i++){
                    createUniformBinding(uniformArray[i]);
                }

                if (typeof callback == "function") {
                    callback.call(self, program, uniforms);
                }
            });

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
            this.requestPaint();
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

}

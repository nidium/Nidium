/*
 * Copyright 2017 Nidium Inc. All rights reserved.
 * Use of this source code is governed by a MIT license
 * that can be found in the LICENSE file.
 */

const Elements          = require("Elements");
const VM                = require("VM");
const s_ComponentShadow = require("../Symbols.js").ComponentShadowRoot;
const s_ShadowHost      = require("../Symbols.js").ElementShadowHost;
const s_ShadowRoot      = require("../Symbols.js").ElementShadowRoot;
const { StyleContainer, ElementStyle } = require("ElementsStyles");

function findUserSlots(children, ret) {
    if (!ret) ret = new Map();

    for (let child of children) {
        if (child.attributes && child.attributes.slot) {
            ret.set(child.attributes.slot, child);
        }
        if (child.children) {
            findUserSlots(child.children, ret);
        }
    }

    return ret;
}

class Component extends Elements.Element {
    constructor(attributes = {}) {
        super(attributes);

        this.attachShadow({
            "name": "ComponentInstance-" + this.name(),
            "scope": this.constructor[s_ComponentShadow].getJSScope()
        });

        // Share the NSS between the ShadowRoot of the Component and this Element
        this.shadowRoot.nss = this.constructor[s_ComponentShadow].getNSS();

        this.on("mount", () => {
            this.mounted = true;
            this.updateNSS();
            this.createComponentTree();
            if (this.children) {
                this.createTree(this.children);
            }
            this.fireEvent("ready", {});
        });
    }

    updateNSS() {
        /*
            Merge & Apply style components
            XXX : This should be refactored to avoid code duplication with element.js
        */
        const tmp = [];
        const layout = this.constructor[s_ComponentShadow].layout;

        // Apply style defined on <layout> to the component
        const classes = layout.attributes.class;
        if (classes) {
            const nss = this.shadowRoot.getNSS();

            // Add all NSS style defined by every classes
            for (let c of classes.split(" ")) {
                tmp.push(nss[c]);
            }
        }

        // Give priority to class defined by the user
        if (this.attributes.class) {
            let nss;
            const parent = this.getParent();
            if (parent.shadowRoot) {
                nss = parent.shadowRoot.getNSS();
            } else {
                nss = this.getParent()[s_ShadowRoot].getNSS();

            }

            for (let c of this.attributes.class.split(" ")) {
                tmp.push(nss[c]);
            }
        }

        // Also parse style attributes
        if (this.attributes.style) {
            var style = VM.run("(" + this.attributes.style + ")", {
                scope: this[s_ShadowRoot].getJSScope()
            });
            tmp.push(style);
        }

        // Merge all styles, into |this.style|
        this._mergeStyle(tmp);
    }

    createTree(children) {
        if (!this.mounted && !this.children) {
            this.children = children;
            return;
        }

        /*
            Create & Add children to the components
        */
        if (!this.allowsChild()) {
            if (children.length) {
                console.warn(`Component <${this.name()}> does not allow children. Ignoring children.`);
            }
            return;
        }

        if (typeof(children) == "undefined" || children.length == 0) {
            return;
        }

        // Slot handling
        let parentShadow    = this[s_ShadowRoot];
        let shadowSlots     = this.shadowRoot.findNodesByTag("slot");
        let userSlots       = findUserSlots(children);

        if (shadowSlots.length == 0 && children.length > 0) {
            console.warn(`Component <${this.name()}> does not expose any slot. Ignoring children.`);
            return;
        }

        // Add the child before the <slot> tag inside
        // the Component, and keep the child ShadowRoot
        let replace = (node, ...childs) => {
            const p = node.getParent();
            if (!p) {
                throw new Error("<slot> doen't have a parent. This is not possible.");
            }

            for (let child of childs) {
                p.insertAfter(child, node);
            }

            node.removeFromParent();
        }

        if (userSlots.size == 0) {
            // Use slot as default slot
            replace(shadowSlots[0], ...NML.CreateTree(children, null, parentShadow));
        } else {
            for (let [id, el] of userSlots) {
                for (let slot of shadowSlots) {
                    if (slot.id == id) {
                        replace(slot, ...NML.CreateTree([el], null, parentShadow));
                    }
                }
            }
        }
    }

    createComponentTree() {
        const layout    = this.constructor[s_ComponentShadow].layout;
        const templates = this.constructor[s_ComponentShadow].template;
        const scope     = this.shadowRoot.getJSScope();

        // When rendering a component, |this| should be the instance of the component
        const previousThis = scope["this"];
        scope["this"] = this;

        try  {
            if (layout) {
                // Render layout
                NML.CreateTree(layout.children, this, this.shadowRoot);
            } else if (templates.length == 1) {
                // XXX : Update me
                // Render template
                //this.addMultiple(...templates[0].render(scope));
                throw new Error("<template> tag is not yet implemented");
            }
        } finally {
            scope["this"] = previousThis;
        }
    }
}

ElementStyle.Inherit(Component);

module.exports = Component;

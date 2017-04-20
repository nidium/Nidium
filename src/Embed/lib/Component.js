/*
 * Copyright 2017 Nidium Inc. All rights reserved.
 * Use of this source code is governed by a MIT license
 * that can be found in the LICENSE file.
 */

const Elements          = require("Elements");
const s_ComponentShadow = require("../Symbols.js").ComponentShadowRoot;
const s_ShadowHost      = require("../Symbols.js").ElementShadowHost;
const s_ShadowRoot      = require("../Symbols.js").ElementShadowRoot;

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

        this.createComponentTree();
    }

    createTree(children) {
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

        // Add the child inside the Component, but keep the child ShadowRoot
        let adopt = (p, ...childs) => {
            p.empty();
            for (let child of childs) {
                Canvas.prototype.add.call(p, child);
            }
        }

        if (userSlots.size == 0) {
            // Use slot as default slot
            shadowSlots[0].empty();
            adopt(shadowSlots[0], ...NML.CreateTree(children, null, parentShadow));
        } else {
            for (let [id, el] of userSlots) {
                for (let slot of shadowSlots) {
                    if (slot.id == id) {
                        adopt(slot, ...NML.CreateTree([el], null, parentShadow));
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
                // Apply style defined on <layout> to the component
                const classes = layout.attributes.class;
                if (classes) {
                    const tmp = [];
                    const nss = this.shadowRoot.getNSS();

                    for (let c of classes.split(" ")) {
                        tmp.push(nss[c]);
                    }

                    //tmp.push(Object.assign({}, this.style));

                    tmp.unshift(this.style);

                    Object.assign.apply(null, tmp);
                }

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

module.exports = Component;

/*
 * Copyright 2017 Nidium Inc. All rights reserved.
 * Use of this source code is governed by a MIT license
 * that can be found in the LICENSE file.
 */

{
    const Elements          = require("Elements");
    const Component         = require("Component");
    const s_ComponentShadow = require("../../Symbols.js").ComponentShadowRoot;
    const s_ComponentName   = Symbol("ComponentName");
    const s_ComponentClass  = Symbol("ComponentClass");
    const { StyleContainer, ElementStyle } = require("ElementsStyles");

    Object.defineProperty(window.require, "ComponentLoader", {
        "configurable": false,
        "writable": false,
        "value": function(name, lst) {
            let ret = {};
            let counter = 0;
            for (let el of lst) {
                if (el.type != "component") {
                    throw new Error("Only <component> can be loaded with require()");
                }

                let tmp = NML.CreateTree(lst);
                let c = tmp[0];

                ret[this[s_ComponentName]] = c.getComponentClass();
                counter++;
            }

            // Export one or more component
            return counter == 1 ? ret[Object.keys(ret)[0]] : ret;
        }
    });

    class ComponentExports {}

    Elements.component = class extends Elements.Node {
        constructor(attributes) {
            super(attributes);

            this.hide();

            if (attributes.src) {
                this.isLoader = true;
                NML.CreateTree(Elements.Loader(attributes));
                return;
            }

            if (!attributes.name) {
                throw new Error("<component> must have a name attribute");
            }

            this[s_ComponentName] = attributes.name;

            let scope = {
                Component: Component,
                "module": {"exports": new ComponentExports()},
                "document": new Proxy(document, {
                    get: (object, key, proxy) => {
                        switch (key) {
                            case "getElementsByTagName":
                                return (id) => {
                                    return this.shadowRoot.findNodesByTag(id);
                                }
                            case "getElementById":
                                return (id) => {
                                    return this.shadowRoot.findNodeById(id);
                                }
                            default:
                                var value = typeof(document[key]) == "function" ?
                                    document[key].bind(document) : document[key];
                                    
                                return value;
                        }
                    },

                    set: (object, key, value, proxy) => {
                        throw new Error(`Components are not allowed to set document.${key}`);
                    }
                })
            };

            /*
            scope["window"] = new Proxy(scope, {
                get: (object, key, proxy) => {
                    switch (key) {
                        case "addEventListener":
                        case "fireEvent":
                            return window[key];
                        default:
                            return window[key];
                    }
                },

                set: (object, key, value, proxy) => {
                    throw new Error(`Components are not allowed to set window.${key}`);
                }
            });
            */

            // Prevent access to the global object using |this|
            scope["this"] = scope.window;

            // XXX : Components can still access variables/methods from
            // the global object, maybe using a sandbox is more suited ?

            this.attachShadow({"scope": scope, "name": "Component-" + this.tagName});
        }

        get tagName() {
            return this[s_ComponentName];
        }

        getComponentClass() {
            return this[s_ComponentClass];
        }

        createTree(children) {
            if (this.isLoader) {
                // Component(s) loaded from src="" nothing to do
                return;
            }

            let i = children.length;
            while (i--) {
                const child = children[i];

                switch (child.type) {
                    case "layout":
                        this.shadowRoot.layout = child;
                        children.splice(i, 1);
                        break;
                    case "template":
                        this.shadowRoot.template = child;
                        children.splice(i, 1);
                        break;

                }
            }

            let ret     = super.createTree(children);
            let scope   = this.shadowRoot.getJSScope()

            let componentClass = scope.module.exports;

            if (componentClass instanceof ComponentExports) {
                if (Object.keys(componentClass).length > 0) {
                    throw new Error("You cannot export more than one Component");
                } else {
                    console.info(`Missing JavaScript implementation for component "${this.tagName}". Using default component.`);
                }

                componentClass = eval("(function() { return class extends Component {} })()");
            }

            if (!Component.isPrototypeOf(componentClass)) {
                throw new Error(`Failed to load component "${this.name}". Only class extending Component can be exported.`);
            }

            ElementStyle.Inherit(componentClass);

            this[s_ComponentClass] = componentClass;

            // Store a reference to the original shadowRoot of the component
            // so the instance of the component can retrieve the layout/templates
            // it needs to build the component.
            componentClass[s_ComponentShadow] = this.shadowRoot;

            Elements[this.tagName] = componentClass;
        }
    }
}

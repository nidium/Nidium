{
    const Elements          = require("Elements");
    const Component         = require("Component");
    const s_ComponentShadow = require("../../Symbols.js").ComponentShadowRoot;
    const s_ComponentName   = Symbol("ComponentName");
    const s_ComponentClass  = Symbol("ComponentClass");

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

                ret[c.name()] = c.getComponentClass();
                counter++;
            }

            // Export one or more component
            return counter == 1 ? ret[Object.keys(ret)[0]] : ret;
        }
    });

    class DefaultComponent extends Component {}
    class ComponentExports {}

    Elements.component = class extends Elements.Node {
        constructor(attributes) {
            super(attributes);

            if (!attributes.name) {
                throw new Error("<component> must have a name attribute");
            }

            this[s_ComponentName] = attributes.name;

            this.hide();

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
                                return object[key];
                                //throw new Error(`Components does not have access to document.${key}`);
                        }
                    },

                    set: (object, key, value, proxy) => {
                        throw new Error(`Components are not allowed to set document.${key}`);
                    }
                }),
                "window": new Proxy(this, {
                    get: (object, key, proxy) => {
                        switch (key) {
                            case "addEventListener":
                            case "fireEvent":
                                return window[key];
                            /*
                            default:
                                throw new Error(`Components does not have access to window.${key}`);
                            */
                        }

                        return object[key];
                    },

                    set: (object, key, value, proxy) => {
                        throw new Error(`Components are not allowed to set window.${key}`);
                    }
                })
            }

            // Prevent access to the global object using |this|
            scope["this"] = window;

            // XXX : Components can still access variables/methods from
            // the global object, maybe using a sandbox is more suited ?

            this.attachShadow({"scope": scope, "name": "Component-" + this.name});
        }

        name() {
            return this[s_ComponentName];
        }

        getComponentClass() {
            return this[s_ComponentClass];
        }

        createTree(children) {
            let ret     = super.createTree(children);
            let scope   = this.shadowRoot.getJSScope()

            let componentClass = scope.module.exports;
            if (componentClass instanceof ComponentExports) {
                if (Object.keys(componentClass).length > 0) {
                    throw new Error("You cannot export more than one Component");
                } else {
                    console.info(`JavaScript implementation for component "${this.name}" not found. Using default component.`);
                }
                componentClass = DefaultComponent;
            }

            if (!Component.isPrototypeOf(componentClass)) {
                throw new Error(`Failed to load component "${this.name}" only class extending Component can be exported`);
            }

            this[s_ComponentClass] = componentClass;

            // Store a reference to the original shadowRoot of the component
            // so the instance of the component can retrieve the layout/templates
            // it needs to build the component.
            componentClass[s_ComponentShadow] = this.shadowRoot;

            Elements[this.name()] = componentClass;
        }
    }
}

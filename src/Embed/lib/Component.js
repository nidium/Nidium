const Elements          = require("Elements");
const s_ComponentShadow = require("../Symbols").ComponentShadowRoot;

class Component extends Elements.Element {
    constructor(attributes) {
        super(attributes);

        this.attachShadow({"name": "ComponentInstance-" + this.name()});

        // Share the NSS between the ShadowRoot of the Component and this Element
        this.shadowRoot.nss = this.constructor[s_ComponentShadow].getNSS();
    }

    name() {
        return this._name;
    }

    createTree(children) {
        let layout      = this.constructor[s_ComponentShadow].findNodesByTag("layout")
        let templates   = this.constructor[s_ComponentShadow].findNodesByTag("template");

        if (layout) {
            for (let child of layout[0].getChildren()) {
                this.add(child.cloneNode(true, this.shadowRoot));
            }
        } else if (templates && templates.length == 1) {
            // When rendering templates, |this| should be the instance of the component
            let scope = this.constructor[s_ComponentShadow].getJSScope();
            let previousThis = scope["this"];

            scope["this"] = this;

            try {
                this.addMultiple(...templates[0].render(scope));
            } finally {
                scope["this"] = previousThis;
            }
        }
    }
}

module.exports = Component;

const VM            = require("VM");
const Elements      = require("Elements");
const Component     = require("Component");

const ResourceType = {
    NML: 0,
    JS: 1,
    NSS: 2
};

class DefaultComponent extends Component {}

class ComponentExports {}

class ComponentLoader {
    constructor(filename, nml) {
        this.filename = filename;
        this.nss = [];
        this.scripts = [];
        this.template = null;
        this.layout = null;

        this.parse(nml);
    }

    add(data, filename, type) {
        let key = (type == ResourceType.NSS ? "nss" : "scripts");

        this[key].push({
            "data": data,
            "filename": filename,
            "line": 1,
            "type": type
        });
    }

    parse(c) {
        if (c.type != "component") {
            throw new Error("Top level tag should be <component>");
        }

        if (!c.attributes || !c.attributes.name) {
            throw new Error("No name attribute for the component");
        }

        this.name = c.attributes.name;

        for (let child of c.children) {
            switch (child.type) {
                case "nss":
                    this.add(child.text, `${this.filename} (inline <nss> #${this.nss.length})`, ResourceType.NSS);
                    break;
                case "script":
                    this.add(child.text, `${this.filename} (inline <script> #${this.scripts.length})`, ResourceType.JS);
                    break;
                case "template":
                    if (this.template) throw new Error("Only one <template> tag is allowed");
                    this.template = child;
                    break;
                case "layout":
                    if (this.layout) throw new Error("Only one <layout> tag is allowed");
                    this.layout = child;
                    break;
            }
        }

        if (this.template && this.layout) {
            throw new Error("Either <template> or <layout> must be defined, not both.");
        }

        this.registerComponent(c);
    }

    registerComponent(nml) {
        let scope = {Component: Component, "module": {"exports": new ComponentExports()}};
        for (let script of this.scripts) {
            VM.run(script.data, {"scope": scope});
        }

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

        Component.register(this.name, componentClass, {
            "scope": scope,
            "nss": this.nss,
            "nml": nml,
            "template": this.template,
            "layout": this.layout,
        });

        this.component = componentClass;
    }

    getComponent() {
        return this.component;
    }

    getName() {
        return this.name;
    }
}

module.exports = ComponentLoader;

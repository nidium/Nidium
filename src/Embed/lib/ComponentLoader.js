const VM            = require("VM");
const Elements      = require("Elements");
const Component     = require("Component");
const s_Priv        = require("../ComponentsPrivate.js");

const ResourceType = {
    NML: 0,
    JS: 1,
    NSS: 2
};

class DefaultComponent extends Component {}

class ComponentLoader {
    constructor(nml, callback) {
        this.nml = nml;
        this.callback = callback;
        this.nss = [];
        this.scripts = [];
        this.template = null;
        this.layout = null;

        let data = File.readSync(nml, {encoding: "utf-8"});
        this.parse(data);
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

    parse(data) {
        let ret = NML.parse(data);
        if (!ret) throw new Error("Failed to parse NML");

        let c = ret[0];
        if (c.type != "component") {
            throw new Error("Top level tag should be <component>");
        }

        if (!c.attributes || !c.attributes.name) {
            throw new Error("No name attribute for the component");
        }

        for (let child of c.children) {
            switch (child.type) {
                case "nss":
                    this.add(child.text, `${this.nml} (inline <nss> #${this.nss.length})`, ResourceType.NSS);
                    break;
                case "script":
                    this.add(child.text, `${this.nml} (inline <script> #${this.scripts.length})`, ResourceType.JS);
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
        let scope = {Component: Component};
        let name = nml.attributes.name;

        for (let script of this.scripts) {
            VM.run(`${script.data}\nComponent.${name} = ${name};`, {"scope": scope});
        }

        let componentClass = Component[name]
        if (typeof(componentClass) == "undefined") {
            console.info(`Component "${name}" not found. Using default component.`);
            componentClass = DefaultComponent;
            Component[name] = componentClass;
        }

        Elements[name.toLowerCase()] = componentClass;
        componentClass[s_Priv] = {
            "scope": scope,
            "nss": this.nss,
            "nml": nml,
            "template": this.template,
            "layout": this.layout,
        }

        this.callback(null);
    }
}

module.exports = ComponentLoader;

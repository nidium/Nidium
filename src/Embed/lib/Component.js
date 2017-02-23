const Elements       = require("Elements");
const TemplateEngine = require("TemplateEngine");
const VM             = require("VM");
const s_Priv         = Symbol("ComponentPrivate")

const ResourceType = {
    NML: 0,
    JS: 1,
    NSS: 2
};

function componentInit() {
    let privates = this.constructor[s_Priv];

    /*
        Load & Merge NSS
    */
    if (privates && privates.nml) {
        let processedNSS = [];
        for (let i = 0; i < privates.nss.length; i++) {
            let style = privates.nss[i]
            let scope = privates.scope;

            scope.__parseNSS = function processNSS(NSS, data) {
                NSS.push(data);
            }.bind(this, processedNSS);

            VM.run(`__parseNSS({${style.data}})`, {
                "scope": privates.scope,
                "filename": style.filename,
            });

            delete scope.__parseNSS;
        }

        // Merge all styles into |this.nss|
        processedNSS.unshift(this.nss);
        Object.assign.apply(null, processedNSS);

        // And make them read-only
        Object.freeze(this.nss);
    }

    /*
        Load & Process template
    */
    if (privates && privates.template) {
        let tpl = new Elements.template(privates.template);
        privates.scope["this"] = this;

        this.layout = tpl.render(privates.scope);

        delete privates.scope["this"];

        if (!this.layout) {
            console.error("Failed to parse template");
            //throw new Error("Failed to parse template");
        }
    }

    /*
        Render layout
    */
    if (privates && privates.layout) {
        this.layout = privates.layout;
    }

    if (this.layout) {
        let setComponent = (el) => {
            let childs = el.getChildren();
            el.component = this;
            for (let child of childs) {
                if (!(child instanceof Component)) {
                    child.component = this;
                    setComponent(child);
                }
            }
        }

        for (el of this.layout) {
            this.add(el);
            setComponent(el);
        }
    }

    this.fireEvent("ready", {});
}

class Component extends Elements.Element {
    constructor(attributes) {
        super(attributes);

        this.nss = {};
        this._name = this.constructor.name;

        // Delay real init to allow the extending
        // component to do it's initial setup
        setImmediate(componentInit.bind(this));
    }

    name() {
        return this._name;
    }

    static register(name, cls, privates) {
        Component[name] = cls;
        Elements[name.toLowerCase()] = cls;

        cls[s_Priv] = privates;
    }
}

module.exports = Component;

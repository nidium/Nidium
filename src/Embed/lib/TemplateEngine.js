const Elements = require("Elements");

const g_Engines = {}
let g_DefaultEngine = null;

Elements.template = class extends Elements.Node {
    constructor(node) {
        super(node);

        let engine = g_DefaultEngine;

        if (node.attributes && node.attributes.type &&
                !(engine = TemplateEngine.get(node.attributes.type))) {
            console.warn(`Template compiler ${node.attributes.type} not found. Using no-op compiler.`);
            engine = g_DefaultEngine;
        }

        return new engine(Elements.Loader(node), node.attributes);
    }

    // Override
    isAutonomous() {
        return true;
    }
}

class TemplateEngine {
    constructor(data, attributes) {}

    render(data) {
        return NML.CreateTree(NML.parse(data));
    }

    static register(name, impl) {
        if (g_Engines[name]) {
            throw new Error(`Template engine ${name} is already registred`);
        }

        if (!g_DefaultEngine) {
            g_DefaultEngine = impl;
        }

        g_Engines[name] = impl;
    }

    static get(engineName) {
        return g_Engines[engineName];
    }
}

/*
    Default template engine (nunjucks)
*/
const Nunjucks = require("nunjucks");

class NunjucksTemplate extends TemplateEngine {
    constructor(data, attributes) {
        super(data, attributes);
        this.tpl = Nunjucks.compile(data);
    }

    render(scope) {
        return super.render(this.tpl.render(scope));
    }
}

TemplateEngine.register("nunjucks", NunjucksTemplate);

module.exports = TemplateEngine

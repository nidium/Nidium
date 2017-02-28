const g_Engines     = {}
const Elements      = require("Elements");
const s_Node        = Symbol("TemplateNode");
let g_DefaultEngine = null;

class TemplateEngine {
    constructor(node, attributes) {
        if (!node || !(node instanceof Elements.template)) {
            throw new Error("TemplateEngine constructor called without an element or with an element that is not a <template>");
        }

        this[s_Node] = node;
    }

    compile(data) { }

    render(data) {
        return NML.CreateTree(data, this.getNode().getRootNode().shadowRoot);
    }

    getNode() {
        return this[s_Node];
    }

    static Register(name, impl) {
        if (g_Engines[name]) {
            throw new Error(`Template engine ${name} is already registred`);
        }

        if (!g_DefaultEngine) {
            g_DefaultEngine = impl;
        }

        g_Engines[name] = impl;
    }

    static Get(engineName) {
        return g_Engines[engineName];
    }

    static GetDefaultEngine() {
        return g_DefaultEngine;
    }
}

/*
    Default template engine (nunjucks)
*/
const Nunjucks = require("nunjucks");

TemplateEngine.Register("nunjucks", class extends TemplateEngine {
    compile(data) {
        this.tpl = Nunjucks.compile(data);
    }

    render(scope) {
        return super.render(this.tpl.render(scope));
    }
});

module.exports = TemplateEngine

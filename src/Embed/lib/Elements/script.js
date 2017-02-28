{
    const VM            = require("VM");
    const Elements      = require("Elements");
    const s_ShadowRoot  = require("../../Symbols.js").ElementShadowRoot;

    Elements.script = class extends Elements.Node {
        constructor(attributes) {
            super(attributes);
            this.hide();
            if (attributes.src) {
                this.data = Elements.Loader(attributes);
            }
        }

        createTree(children) {
            let code = this.data;
            if (!code) {
                code = children[0].text;
            }
            VM.run(code, {"scope": this[s_ShadowRoot].getJSScope()})
        }
    }
}

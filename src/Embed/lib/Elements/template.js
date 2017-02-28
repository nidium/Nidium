{
    const Elements          = require("Elements");
    const TemplateEngine    = require("TemplateEngine");

    Elements.template = class extends Elements.Node {
        constructor(attributes) {
            super(attributes);

            let engine = TemplateEngine.GetDefaultEngine();

            if (attributes && attributes.type &&
                    !(engine = TemplateEngine.Get(attributes.type))) {
                console.warn(`Template compiler ${attributes.type} not found. Using default compiler.`);
                engine = TemplateEngine.GetDefaultEngine();
            }

            if (attributes.src) {
                this.data = Elements.Loader(attributes);
            }

            this.engine = new engine(this, attributes);
        }

        createTree(children) {
            this.data = this.data || children[0].text;
            this.engine.compile(this.data);
        }

        compile(data) {
            return this.engine.compile(data);
        }

        render(scope) {
            return this.engine.render(scope);
        }
    }
}

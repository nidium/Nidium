{
    const Elements = require("Elements");

    Elements.layout = class extends Elements.Node {
        constructor(attr) {
            super(attr)
            this.addEventListener("load", () => {
                // Expand layout to take full width of it's parent
                // to allow correct sizing of it's children
                this.width = this.getParent().width;
            });
        }
    }
}

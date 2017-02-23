{
    const ComponentLoader = require("ComponentLoader");
    const Elements = require("Elements");

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
                let loader = new ComponentLoader(name, el);
                ret[loader.getName()] = loader.getComponent();
                counter++;
            }

            // Export one or more component
            return counter == 1 ? ret[Object.keys(ret)[0]] : ret;
        }
    });

    Elements.component = class extends Elements.Node {
        constructor(node) {
            super(node);

            let name = node.attributes && node.attributes.src ? node.attributes.src : "inline";
            let data = Elements.Loader(node);

            new ComponentLoader(node, node);
        }

        isAutonomous() {
            return true;
        }
    }
}

require("./components/button.nc");
require("./components/square.nc");

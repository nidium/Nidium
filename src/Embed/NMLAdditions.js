/*
   Copyright 2017 Nidium Inc. All rights reserved.
   Use of this source code is governed by a MIT license
   that can be found in the LICENSE file.
*/
{
    const Elements = require("Elements");

    function walk(elems, shadowRoot) {
        if (!elems) {
            return [];
        }

        let ret = [];

        for (let elem of elems) {
            let name = elem.type;

            /*
               ES6 destructuring object default value doesnt work
               https://bugzilla.mozilla.org/show_bug.cgi?id=932080
            */
            let {id} = elem.attributes || {id: null};
            let ui = Elements.Create(elem.type, elem.attributes || elem.text, shadowRoot);

            if (id) {
                ui.id = id;
            }

            ret.push(ui);

            ui.createTree(elem.children);
        }

        return ret;
    }

    NML.CreateTree = function(nml, shadowRoot) {
        let tree;
        if (typeof(nml) == "string") {
            tree = NML.parse(nml);
            if (!tree) {
                /* If the NML couldnt be parsed, issue a textnode */
                return [Elements.Create("textnode", nml, shadowRoot)];
            }
        } else if (!Array.isArray(nml)) {
            throw new Error("NML.CreateTree expect an array");
        } else {
            tree = nml;
        }
        return walk(tree, shadowRoot);
    }
}

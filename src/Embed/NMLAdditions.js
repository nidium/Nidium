/*
   Copyright 2016 Nidium Inc. All rights reserved.
   Use of this source code is governed by a MIT license
   that can be found in the LICENSE file.
*/
{
    const Elements = require("Elements");

    function walk(elems, parent) {
        if (!elems) {
            return [];
        }

        let ret = [];

        for (let elem of elems) {
            let name = elem.type;
            if (Elements.Exists(name)) {
                if (Elements.Element.prototype.isPrototypeOf(Elements[name.toLowerCase()].prototype)) {
                    /* ES6 destructuring object default value doesnt work
                       https://bugzilla.mozilla.org/show_bug.cgi?id=932080
                    */
                    let {id} = elem.attributes || {id: null};

                    ui = Elements.Create(elem.type, elem.attributes || elem.text);

                    if (id) {
                        ui.id = id;
                    }

                    if (parent) {
                        parent.add(ui);
                    } else {
                        ret.push(ui);
                    }
                } else {
                    let tmp = new Elements[name](elem, parent);
                    if (tmp.autonomous()) {
                        return;
                    }
                }
            } else {
                throw new Error(`Unknown element <${name}>`);
            }

            walk(elem.children, ui);
        }

        return ret;
    }

    NML.CreateTree = function(nml) {
        let tree;
        if (typeof(nml) == "string") {
            tree = NML.parse(nml);
            if (!tree) {
                /* If the NML couldnt be parsed, issue a textnode */
                return [Elements.Create("textnode", nml)];
            }
        } else {
            tree = nml;
        }

        return walk(tree);
    }
}

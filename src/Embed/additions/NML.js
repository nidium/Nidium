/*
   Copyright 2017 Nidium Inc. All rights reserved.
   Use of this source code is governed by a MIT license
   that can be found in the LICENSE file.
*/
{
    const Elements = require("Elements");

    function walk(elements, parent, shadowRoot) {
        if (!elements) {
            return [];
        }

        let ret = [];

        for (let node of elements) {
            let name = node.type;

            /*
               ES6 destructuring object default value doesnt work
               https://bugzilla.mozilla.org/show_bug.cgi?id=932080
            */
            let {id} = node.attributes || {id: null};

            let el = Elements.Create(
                node.type, node.attributes || node.text, shadowRoot
            );

            if (parent) {
                parent.add(el);
            } else {
                ret.push(el);
            }

            el.createTree(node.children);
        }

        return ret;
    }

    NML.CreateTree = function(nml, parent, shadowRoot) {
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
        return walk(tree, parent, shadowRoot);
    }
}

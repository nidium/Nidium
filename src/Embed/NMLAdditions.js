/*
   Copyright 2016 Nidium Inc. All rights reserved.
   Use of this source code is governed by a MIT license
   that can be found in the LICENSE file.
*/
let Elements = require("./lib/Elements.js");
{
    NML.CreateTree = function(nml)
    {
        if (typeof(nml) == "string") {
            var tree = NML.parse(nml);
            if (!tree) {
                /* If the NML couldn't be parsed, issue a textnode */
                return [Elements.Create("textnode", nml)];
            }
        } else {
            var tree = nml;
        }

        function walk(elems, parent) {
            if (!elems) {
                return [];
            }

            var ret = [];

            for (let elem of elems) {

                /* ES6 destructuring object default value doesnt work
                   https://bugzilla.mozilla.org/show_bug.cgi?id=932080
                */
                let {id} = elem.attributes || {id: null};

                var ui = Elements.Create(elem.type, elem.attributes || elem.text);

                if (id) {
                    ui.id = id;
                }

                if (parent) {
                    parent.add(ui);
                } else {
                    ret.push(ui);
                }

                walk(elem.children, ui);
            }

            return ret;
        }

        return walk(tree);

    }

}

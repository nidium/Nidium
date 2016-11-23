/*
   Copyright 2016 Nidium Inc. All rights reserved.
   Use of this source code is governed by a MIT license
   that can be found in the LICENSE file.
*/

NML.CreateTree = function(nml)
{
    if (typeof(nml) == "string") {
        var tree = NML.parse(nml);
        if (!tree) {
            return [];
        }
    } else {
        var tree = nml;
    }

    function walk(elems, parent) {
        var ret = [];

        for (let elem of elems) {
            if (!(elem.type in Elements)) {
                continue;
            }

            /* ES6 destructuring object default value doesnt work
               https://bugzilla.mozilla.org/show_bug.cgi?id=932080
            */
            let {id} = elem.attributes;

            var ui = new Elements[elem.type](elem.attributes);

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

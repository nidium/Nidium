/* -------------------------------------------------------------------------- */
/* MIT license                                          (c) 2016 Nidium, Inc. */
/* -------------------------------------------------------------------------- */
/* Permission is hereby granted, free of charge, to any person obtaining a    */
/* copy of this software and associated documentation files (the "Software"), */
/* to deal in the Software without restriction, including without limitation  */
/* the rights to use, copy, modify, merge, publish, distribute, sublicense,   */
/* and/or sell copies of the Software, and to permit persons to whom the      */
/* Software is furnished to do so, subject to the following conditions:       */
/*                                                                            */
/* The above copyright notice and this permission notice shall be included in */
/* all copies or substantial portions of the Software.                        */
/*                                                                            */
/* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR */
/* IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,   */
/* FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL    */
/* THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER */
/* LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING    */
/* FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER        */
/* DEALINGS IN THE SOFTWARE.                                                  */
/* -------------------------------------------------------------------------- */


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
            let {id, width, height} = elem.attributes;
            width = width || 50;
            height = height || 50;

            var ui = new Elements[elem.type](width, height, elem.attributes);
            ui.id = id;

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

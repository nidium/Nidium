/* -------------------------------------------------------------------------- */
/* MIT license                                          (c) 2017 Nidium, Inc. */
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

const propMatch = /{{(?:\s*?)([^\s:]+?)(?:\s*?)}}/g;

class MVVM {

    constructor(obj) {
        this.globalProps = new Map();

        this.data = this._createStore(obj.data);
    }

    _createStore(obj) {
        var that = this;
        
        return new Proxy(obj, {
            set(target, property, value, rcv) {
                target[property] = value;

                var map = that.globalProps.get(property) || []

                for (let data of map) {
                    data.obj._updateAttribute(data.attr);
                }
            },

            get(target, property, rcv) {
                return target[property];
            }
        });
    }

    linkObjectAttr(obj, property, attr){
        var map = this.globalProps.get(property);
        var data = {attr, obj};

        if (!map) {
            map = [];
            this.globalProps.set(property, map)
        }

        map.push({attr, obj});
    }
}

/*
    Process the attributes list when a node is mounted (inserted in the tree)
*/
Canvas.prototype.onmount = function()
{
    this._initAttributes();
}

Canvas.prototype._initAttributes = function()
{
    if (!this.attributes) {
        return;
    }

    var vm = this.getVM();

    if (!vm) {

        return;
    }

    var match;

    for (let attr of Object.getOwnPropertyNames(this.attributes)) {
        let value = this.attributes[attr];
        while (match = propMatch.exec(value)) {

            let prop = match[1];

            vm.linkObjectAttr(this, prop, attr);

        }

        this._setTemplatedAttribute(attr, value);
    }
}

Canvas.prototype.setAttribute = function(attr, value) {

}

Canvas.prototype._setTemplatedAttribute = function(attr, value)
{
    /* TODO: cache VM */
    var vm = this.getVM();

    if (!vm) {
        return;
    }

    let final = value.replace(propMatch, function(match, p1) {
        return vm.data[p1] || match;
    });

    this.setAttribute(attr, final);
}

/*
    Tell the Canvas that a data has changed in a given attribute
    This will recompute the entire attribute value using the initial template
*/
Canvas.prototype._updateAttribute = function(attr)
{
    this._setTemplatedAttribute(attr, this.attributes[attr]);
}

/*
    Bind the View model to this object
    (this will affect itself and all the children)
*/
Canvas.prototype.bindVM = function(vm)
{
    this.inherit._vm = vm;
}

/*
    View model object is bound the the canvas inheritance
*/
Canvas.prototype.getVM = function()
{
    return this.inherit._vm;
}

document.CreateView = function(obj) {
    var vm = new MVVM(obj);
    document.canvas.bindVM(vm);

    return vm;
}

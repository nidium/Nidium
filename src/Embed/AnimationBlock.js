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


var Easing = require("easing");

var AnimationsList = new Set();

{
    let draw = function() {
        let curDate = +new Date();

        for (let anim of AnimationsList) {
            let { end, start, ease, list } = anim;
            let e = ease((curDate - start) / (end - start));
            let finish = (curDate > end);


            for (let elem of list) {
                let { target, property, value, startValue } = elem;
                target[property] = finish ? value : startValue + ((value - startValue) * e);                
            }

            if (finish) {
                AnimationsList.delete(anim);
            }
        }

        window.requestAnimationFrame(draw);
    }

    draw();
}

var AnimationBlock = function(duration, ease, callback, ...objs)
{
    var proxies = [];
    let start = +new Date();

    var anim = {
        end: start+duration,
        ease,
        start,
        list: []
    }

    AnimationsList.add(anim);

    for (let obj of objs) {
        let proxy = new Proxy(obj, {
            set: (target, property, value, rcv) => {
                if (!(property in target)) {
                    return true;
                }

                anim.list.push({
                    startValue: target[property],
                    target,
                    property,
                    value,                 
                })
                
                return true;
            },

            get: (target, prop, rcv) => {
                return target[prop];
            }
        });

        proxies.push(proxy);
    }

    let next = callback(...proxies);
    if (typeof next == 'function') {
        //AnimationBlock(duration, ease, next, )
    }

}

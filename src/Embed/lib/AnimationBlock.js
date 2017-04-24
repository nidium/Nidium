/*
 * Copyright 2017 Nidium Inc. All rights reserved.
 * Use of this source code is governed by a MIT license
 * that can be found in the LICENSE file.
 */

/*
    AnimationBlock provides a way to easily create animation is a declarative way

    Example on animating properties for my_obj and my_obj2 :

        var handler = AnimationBlock((my_obj, my_obj2) => {
            my_obj.left = 200;
            my_obj.top = 50;
            my_obj2.opacity = 0.2;
            my_obj2.left = my_obj.left;

            return () => {
                // chained animation
                
                my_obj.left = 0;
            }
        }, 2000, Easing.Bounce.Out, my_obj, my_obj2);

        handler.onFinish = () => {
            console.log("Animation ended");
        };

    Example on animating properties on a list of objects (array(lst))

        var anim = AnimationBlock((...lst) => {
            for (let o of lst) {
                o.left = Math.random()*600;
                o.top = Math.random()*500;
            }
        }, 2000, Easing.Bounce.Out, ...lst);

        handler = () => {
            console.log("Animation ended");
        };
*/

const Easing = require("easing");

var AnimationsList = new Set();

{
    let draw = function() {
        let curDate = +new Date();

        for (let anim of AnimationsList) {
            let { end, start, ease, list, duration } = anim;
            let e = ease((curDate - start) / (end - start));
            let finish = (curDate > end) || anim.finished;

            for (let elem of list) {
                let { target, property, value, startValue } = elem;
                target[property] = finish ? value : startValue + ((value - startValue) * e);                
            }

            if (finish) {
                if (anim.next) {
                    anim.redo(anim.next);
                } else {
                    anim.onFinish();
                    AnimationsList.delete(anim);
                }
            }
        }

        window.requestAnimationFrame(draw);
    }

    draw();
}

var AnimationBlock = function(callback, duration, ease, ...objs)
{
    let proxies = [];

    var anim = {
        duration,
        ease,
        objs,
        onFinish: function(){},
        finished: false,
        loop: false /* todo */
    };

    AnimationsList.add(anim);

    for (let obj of objs) {
        let proxy = new Proxy(obj, {
            set: (target, property, value, rcv) => {
                if (!(property in target)) {
                    // TODO: Check numeric
                    return true;
                }

                anim.list.push({
                    startValue: target[property] || 0,
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

    (anim.redo = function(callback) {
        let start = +new Date();
        
        /* Reset some state since we're using the same object when chaining */
        Object.assign(anim, {start, end: start+duration, list: [], next: null});

        /* Check whether we have a chained animation */
        let next = callback(...proxies);
        if (typeof next == 'function') {
            anim.next = next;
        }

    })(callback);

    var handler = {
        finish() {
            anim.finished = true;
        },

        cancel() {
            anim.onFinish = function(){};
            AnimationsList.delete(anim);
        },

        set onFinish(call) {
            anim.onFinish = call;
        }
    };

    return handler;
}

module.exports = {
    setAnimation: AnimationBlock,
    Easing: Easing
}
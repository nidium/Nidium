var Easing = require("easing");

var AnimationsList = new Set();

{

    let draw = function() {
        let curDate = +new Date();

        for (let elem of AnimationsList) {
            let { end, start, target, property, value, startValue, ease } = elem;

            if (curDate > end) {
                target[property] = value;
                AnimationsList.delete(elem);

            } else {
                let t = (curDate - start) / (end - start);
                let e = ease(t);

                target[property] = startValue + ((value - startValue) * e);
                // TODO : startValue???
            }
        }

        window.requestAnimationFrame(draw);
    }

    draw();

}

var AnimationBlock = function(duration, ease, callback, ...objs)
{
    var proxies = [];
    for (let obj of objs) {
        let proxy = new Proxy(obj, {
            set: (target, property, value, receiver) => {
                if (!(property in target)) {
                    return true;
                }

                let start = +new Date();

                AnimationsList.add({
                    end: start + duration,
                    start: start,
                    target: target,
                    property: property,
                    value: value,
                    startValue: target[property],
                    ease: ease
                });
                
                return true;
            },

            get: (target, prop, rcv) => {
                return target[prop];
            }
        });

        proxies.push(proxy);
    }

    callback(...proxies);
}
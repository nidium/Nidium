const s_ShadowRoot = require("../../Symbols").ElementShadowRoot;

function proxyStyleSet(el, styles, name, value) {
    let p = Canvas.prototype.getParent.call(el);
    switch (name) {
        case "width":
        case "height":
            if (value && value[value.length -1] == "%") {
                if (!p) break;
                el[name] = p[name] * parseInt(value) / 100;
            } else {
                el[name] = value;
            }
            break;
        case "left":
        case "right":
            if (value && value[value.length -1] == "%") {
                if (!p) break;
                el[name] = p.width * parseInt(value) / 100;
            } else {
                el[name] = value;
            }
            break;
        case "top":
        case "bottom":
            if (value && value[value.length -1] == "%") {
                if (!p) break;
                el[name] = p.height * parseInt(value) / 100;
            } else {
                el[name] = value;
            }
            break;
    }

    styles[name] = value;
}

function refreshStyles(el, styles) {
    for (k of ["width", "height", "left", "top", "bottom", "right"]) {
        if (!styles[k]) continue;
        proxyStyleSet(el, styles, k, styles[k]);
    }
}

class ElementStyles {
    constructor(el) {
        this.el = el;

        el.addEventListener("resize", () => {
            refreshStyles(el, this);
        });

        el.addEventListener("load", () => {
            var classes = el.attributes.class
            if (classes) {
                var nss = this.el[s_ShadowRoot].getNSS()
                var tmp = [];
                for (let c of classes.split(" ")) {
                    tmp.push(nss[c]);
                }
                tmp.unshift(this);
                tmp.push(this);
                Object.assign.apply(null, tmp);
            }

            refreshStyles(el, this);
            // Needed to bypass the shadowroot
            let p = Canvas.prototype.getParent.apply(el);
            p.addEventListener("resize", () => {
                refreshStyles(el, this);
            });
        });

        return new Proxy(this, {
            set: (object, key, value, proxy) => {
                proxyStyleSet(el, object, key, value);
                return true;
            },
        });
    }

    paint(ctx) {
        if (this.background) {
            ctx.fillStyle = this.background;
            ctx.fillRect(0, 0, this.el.width, this.el.height);
        }
    }
}

module.exports = ElementStyles;

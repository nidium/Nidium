const s_ShadowRoot = require("../Symbols").ElementShadowRoot;

function styleProxy(el, key, value) {
    let p = Canvas.prototype.getParent.call(el);

    switch (key) {
        case "width":
        case "height":
            if (value && value[value.length -1] == "%") {
                if (!p) break;
                el[key] = p[key] * parseInt(value) / 100;
            } else {
                el[key] = value;
            }
            break;
    
        case "left":
        case "right":
            if (value && value[value.length -1] == "%") {
                if (!p) break;
                el[key] = p.width * parseInt(value) / 100;
            } else {
                el[key] = value;
            }
            break;
    
        case "top":
        case "bottom":
            if (value && value[value.length -1] == "%") {
                if (!p) break;
                el[key] = p.height * parseInt(value) / 100;
            } else {
                el[key] = value;
            }
            break;
    }
}

function refreshStyles(el, styles) {
    for (let key of ["width", "height", "left", "top", "bottom", "right"]) {
        let value = styles[key];

        if (value) {
            styleProxy(el, key, value);
        }
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
                let nss;
                if (el.shadowRoot) {
                    // If element is a ShadowRoot, we need to get the styling
                    // information from the parent ShadowRoot
                    nss = Canvas.prototype.getParent.apply(el)[s_ShadowRoot].getNSS();
                } else {
                    nss = this.el[s_ShadowRoot].getNSS();
                }

                let tmp = [];
                for (let c of classes.split(" ")) {
                    tmp.push(nss[c]);
                }

                // Gives priority to variables already defined
                tmp.push(Object.assign({}, this));

                // Merge all style into |this|
                tmp.unshift(this);
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
            set: (styles, key, value, proxy) => {
                styleProxy(el, key, value);
                styles[key] = value;
                return true;
            },
        });
    }

    paint(ctx) {
        let borderWidth = this.borderWidth || 1;
        
        if (this.backgroundColor) {
            ctx.fillStyle = this.backgroundColor;
            ctx.fillRect(0, 0, this.el.width, this.el.height);
        }

        if (this.borderColor) {
            ctx.strokeStyle = this.borderColor;
            ctx.strokeRect(0.5, 0.5, this.el.width-1, this.el.height-1);
        }
    }
}

module.exports = ElementStyles;

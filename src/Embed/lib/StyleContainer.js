const s_ShadowRoot = require("../Symbols").ElementShadowRoot;

const inheritedProperties = [
    "color", "textAlign", "lineHeight",
    "fontFamily", "fontSize", "fontWeight"
];

function styleProxy(el, key, val) {
    let p = Canvas.prototype.getParent.call(el),
        value = parseFloat(val);

    /* inherited properties */
    if (inheritedProperties.includes(key)) {
        value = val;
        el.inherit[key] = val;
        el[key] = val;
        return;
    }

    if (p && value && val[val.length -1] == "%") {
        let parsed = value*0.01;

        switch (key) {
            case "width":
            case "height":
            case "minWidth":
            case "minHeight":
            case "maxWidth":
            case "maxHeight":
                el[key] = p[key] * parsed;
                break;
        
            case "left":
            case "right":
            case "paddingLeft":
            case "paddingRight":
            case "marginLeft":
            case "marginRight":
                el[key] = p.width * parsed;
                break;
        
            case "top":
            case "bottom":
            case "paddingTop":
            case "paddingBottom":
            case "marginTop":
            case "marginBottom":
                el[key] = p.height * parsed;
                break;
        }
    } else {
        el[key] = val == "auto" ? val : value;
    }
}

function refreshStyles(el, styles) {
    let list = [
        "width", "height", "minWidth", "minHeight", "maxWidth", "maxHeight", 

        "left", "right", "top", "bottom",
        "paddingLeft", "paddingRight", "paddingTop", "paddingBottom",
        "marginLeft", "marginRight", "marginTop", "marginBottom",
        "staticLeft", "staticRight", "staticTop", "staticBottom",

        "position", "coating",

        "color", "textAlign", "lineHeight",
        "fontFamily", "fontSize", "fontWeight"
    ];

    for (let key of list) {
        let value = styles[key];

        if (value) {
            styleProxy(el, key, value);
        }
    }
}

var drawer = {
    setShadow : function(ctx, style){
        ctx.shadowOffsetX = style.shadowOffsetX;
        ctx.shadowOffsetY = style.shadowOffsetY;
        ctx.shadowColor = style.shadowColor;
        ctx.shadowBlur = style.shadowBlur;
    },

    disableShadow : function(ctx){
        this.setShadow(ctx, {
            shadowOffsetX : 0,
            shadowOffsetY : 0,
            shadowColor : 0,
            shadowBlur : 0
        });
    }
};

class ElementStyles {
    constructor(el) {
        this.el = el;

        el.addEventListener("resize", () => {
            refreshStyles(el, this);
        });

        var classes = el.attributes.class;
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

        el.addEventListener("load", () => {
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
            get: (styles, name) => {
                return this[name];
            },
            has: function(styles, prop) {
                if (prop in styles) { return true; }
                return false;
            }
        });
    }

    paint(ctx) {
        let s = this,
            w = this.el.width,
            h = this.el.height;

        if (s.fontSize) {
            ctx.fontSize = s.fontSize;
        }

        if (s.fontFamily) {
            ctx.fontFamily = s.fontFamily;
        }

        if (s.backgroundColor) {
            drawer.setShadow(ctx, s);
            ctx.fillStyle = s.backgroundColor;
            ctx.fillRect(0, 0, w, h, s.radius || 0);
            drawer.disableShadow(ctx);
        }

        if (s.shadowBlur) {
            ctx.shadowOffsetX = 0;
            ctx.shadowOffsetY = 0;
            ctx.shadowColor = 0;
            ctx.shadowBlur = 0;
        }

        if (s.borderColor && s.borderWidth) {
            let x = 0;
            let y = 0;

            ctx.lineWidth = s.borderWidth;
            ctx.strokeStyle = s.borderColor;

            ctx.strokeRect(
                x - s.borderWidth*0.5,
                y - s.borderWidth*0.5,
                w + s.borderWidth,
                h + s.borderWidth,
                s.radius + s.borderWidth*0.5
            );
        }
    }
}

module.exports = ElementStyles;

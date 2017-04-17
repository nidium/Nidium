const s_ShadowRoot = require("./Symbols").ElementShadowRoot;

const inheritedProperties = [
    "color", "textAlign", "lineHeight",
    "fontFamily", "fontSize", "fontWeight"
];

const redrawingProperties = [
    "backgroundColor", "color", "fontFamily", "fontSize", "fontWeight"
];


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
        var classes = el.attributes.class;

        this.el = el;
        this.style = {};

        var proxyStyle = new Proxy(this.style, {
            set: (styles, key, value, proxy) => {
                if (inheritedProperties.includes(key)) {
                    el.inherit[key] = value;
                }

                if (redrawingProperties.includes(key)) {
                    el.requestPaint();
                }

                styles[key] = value;
                el[key] = value;
                return true;
            },

            get: (styles, name) => {
                return this.style[name];
            },

            has: function(styles, prop) {
                if (prop in styles) { return true; }
                return false;
            }
        });

        if (classes) {
            let nss;
            if (el.shadowRoot) {
                // If element is a ShadowRoot, we need to get the styling
                // information from the parent ShadowRoot
                nss = el.getParent()[s_ShadowRoot].getNSS();
            } else {
                nss = this.el[s_ShadowRoot].getNSS();
            }

            let tmp = [];
            for (let c of classes.split(" ")) {
                tmp.push(nss[c]);
            }

            // Gives priority to variables already defined
            tmp.push(Object.assign({}, this.style));

            // Merge all style into |this|
            tmp.unshift(proxyStyle);
            Object.assign.apply(null, tmp);
        }

        this.style._paint = this.paint.bind(this);

        return proxyStyle;
    }

    paint(ctx, w, h) {
        let s = this.style;

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
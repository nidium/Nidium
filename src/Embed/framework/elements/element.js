/**
 * Copyright 2017 Nidium Inc. All rights reserved.
 * Use of this source code is governed by a MIT license
 * that can be found in the LICENSE file.
 */

{
    const { StyleContainer, ElementStyle } = require("ElementsStyles");
    const { setAnimation, Easing } = require("AnimationBlock");
    const Elements = require("Elements");
    const drawer = require("../core/drawer.js");
    const VM = require("VM");

    const defineStyleProperty = function(target, property, descriptor){
        let styler = new ElementStyle(property, descriptor);
        ElementStyle.Implement(target, styler);
    };
    const s_ShadowRoot  = require("../../Symbols.js").ElementShadowRoot;

    Elements.Element = class extends Elements.Node {
        constructor(attributes = {}) {
            super(attributes);

            if (attributes.opacity !== undefined) {
                this.opacity = attributes.opacity;
            }

            this.style = new (StyleContainer.Create(Object.getPrototypeOf(this)))(this);

            this.style.fontFamily = "Roboto Regular";
            this.style.fontSize = 15;

            const classes = attributes.class;
            if (classes) {
                let nss;
                if (this.shadowRoot) {
                    // If element is a ShadowRoot, we need to get the styling
                    // information from the parent ShadowRoot
                    nss = this.getParent()[s_ShadowRoot].getNSS();
                } else {
                    nss = this[s_ShadowRoot].getNSS();
                }

                let tmp = [];

                // Add all NSS style defined by every classes
                for (let c of classes.split(" ")) {
                    tmp.push(nss[c]);
                }

                // Merge all style into |this.style|
                this._mergeStyle(tmp);
            }

            if (attributes.style) {
                var style = VM.run("(" + attributes.style + ")", {
                    scope: this[s_ShadowRoot].getJSScope()
                });
                Object.assign(this.style, style);
            }
        }

        enableActiveElement() {
            this.__activetime__ = 0;

            var handler = (e) => {
                this.activeElement(e);
            };

            this.on("mousedown", () => {
                this.activeElement();
            });

            document.canvas.on("mouseup", () => {
                this.releaseElement();
            });
        }

        getActiveElement() {
            var dim = this.getDimensions();

            if (!this.__activelayer__) {
                this.__activelayer__ = new Elements.element();
                this.__activelayer__.style.position = "relative";
                this.add(this.__activelayer__);
            }

            this.__activelayer__.style.width = dim.width;
            this.__activelayer__.style.height = dim.height;

            return this.__activelayer__;
        }

        activeElement() {
            this.__activated__ = true;
            this.__activedate__ = +new Date();

            var node = this.getActiveElement();

            if (this.activeAnimation) {
                this.activeAnimation.cancel();
            }

            node.show();
            node.style.opacity = 0.02;
            this.activeAnimation = setAnimation(
                (node) => {
                    node.opacity = 0.12;
                },
                1000,
                Easing.Exponential.In,
                node
            );

            this.__activelayer__ = node;
            this.setActiveElementShader();
        }

        releaseElement() {
            if (!this.__activated__ || !this.__activelayer__) return false;
            this.__activated__ = false;

            var node = this.__activelayer__;
            var time = (+new Date()) - this.__activedate__;

            this.activeAnimation.cancel();

            if (time<100) {
                node.style.opacity = 0.0;
            } else {
                node.style.opacity = 0.75;
                this.activeAnimation = setAnimation(
                    (node) => {
                        node.opacity = 0.0;
                    },
                    800,
                    Easing.Sinusoidal.Out,
                    node
                );

                this.activeAnimation.onFinish = () => {
                    clearInterval(this.activeTimer);
                    this.__activelayer__.hide();
                };
            }
        }

        setActiveElementShader(shader) {
            if (!this.__activelayer__) return false;

            var ctx = this.__activelayer__.getContext("2d");

            var uniforms = ctx.setShader(`
                #ifdef GL_ES
                precision highp float;
                #endif

                #define TAU 6.28318530718
                #define MAX_ITER 3

                uniform int itime;

                float iGlobalTime = float(itime)/80.;

                void main() {
                    float c = 0.9;
                    float inten = .040;

                    float time = iGlobalTime * 1.5+23.0;
                    // uv should be the 0-1 uv of texture...
                    vec2 uv = gl_FragCoord.xy / (1.5 * n_Resolution.xy);
                    vec2 p = mod(uv*TAU, TAU)-60.0;
                    vec2 i = vec2(p);

                    for (int n = 0; n < MAX_ITER; n++) {
                        float t = time * (1.0 - (3.5 / float(n+1)));
                        i = p + vec2(cos(t - i.x) + cos(t + i.y), sin(t - i.y) + cos(t + i.x));
                        c += 1.0/length(vec2(p.x / (cos(i.x+t)/inten),p.y / (cos(i.y+t)/inten)));
                    }

                    c /= float(MAX_ITER);
                    c = 1.07-pow(c, 1.4);
                    vec3 colour = vec3(pow(abs(c), 8.0));
                    colour = clamp(colour + vec3(0.0, 0.0, 0.0), 0.0, 1.0)*0.85;
                    
                    gl_FragColor = vec4(colour, 0.1);
                }
            `);

            this.__activetime__ = 0;
            clearInterval(this.activeTimer);
            this.activeTimer = setInterval(() => {
                uniforms.itime = this.__activetime__++;
            }, 16);
        }

        onload() {
            this._ctx = this.getContext("2d");
        }

        onpaint() {
            if (!this._ctx) {
                return;
            }

            let dimensions = this.getDimensions();

            this._ctx.save();
            this.clear();
            this.paint(this._ctx, dimensions.width, dimensions.height);
            this._ctx.restore();
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
                s.shadowBlur && drawer.setShadow(ctx, s);
                ctx.fillStyle = s.backgroundColor;
                ctx.fillRect(0, 0, w, h, s.radius);
                s.shadowBlur && drawer.disableShadow(ctx);
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

        shader(file) {
            var ctx = this.ctx2d();
            return ctx.setShaderFile(file);
        }

        fadein(duration, callback) {
            var callback = callback || function(){};

            setAnimation((style) => {
                style.opacity = 1;
            }, duration, Easing.Sinusoidal.Out, this.style)(callback);

            return this;
        }

        fadeout(duration, callback) {
            var callback = callback || function(){};

            setAnimation((style) => {
                style.opacity = 0;
            }, duration, Easing.Sinusoidal.In, this.style)(callback);

            return this;
        }

        isVisible() {
            return this.__visible && !this.__outofbound;
        }

        getDrawingBounds() {
            var {width, height} = this.getDimensions();

            return {
                x : 0,
                y : 0,
                w : width,
                h : height
            };
        }

        ctx2d() {
            return this.getContext("2d");
        }

        /* This is needed because Elements.Node forbids getContext() */
        getContext(type="2d") {
            return Canvas.prototype.getContext.call(this, type);
        }
    }

    Object.defineProperty(Elements.Element.prototype, "_mergeStyle", {
        value: function(stylesArray) {
            for (let styles of stylesArray) {
                for (let prop in styles) {
                    if (styles[prop] != undefined) {
                        this.style[prop] = styles[prop]
                    }
                }
            }
        }
    });

    Elements.element = class extends Elements.Element { }
    Elements.none = Elements.Element;

    [
        "opacity",
        "overflow",
        "scrollLeft",
        "scrollTop",
        "allowNegativeScroll",
        "width",
        "coating",
        "height",
        "maxWidth",
        "maxHeight",
        "minWidth",
        "minHeight",
        "position",
        "display",
        "top",
        "left",
        "right",
        "bottom",
        "visible",
        "marginLeft",
        "marginRight",
        "marginTop",
        "marginBottom",
        "paddingLeft",
        "paddingRight",
        "paddingTop",
        "paddingBottom",
        "cursor",
        "flexDirection",
        "flexWrap",
        "justifyContent",
        "alignItems",
        "alignContent",
        "flexGrow",
        "flexShrink",
        "flexBasis",
        "alignSelf",
        "aspectRatio"
    ].forEach(native => {
        defineStyleProperty(Elements.Element, native, { isNative: true });
    });

    const properties = {
        backgroundColor :   { inherit:false,    repaint:true},
        color :             { inherit:true,     repaint:true},
        lineHeight :        { inherit:true,     repaint:true},
        fontFamily :        { inherit:true,     repaint:true},
        fontSize :          { inherit:true,     repaint:true},
        fontWeight :        { inherit:true,     repaint:true},
        textAlign :         { inherit:true,     repaint:true},
    };

    for (var k in properties) {
        defineStyleProperty(Elements.Element, k, properties[k]);
    }
    
    ElementStyle.Inherit(Elements.element);
}

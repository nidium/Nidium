/*
   Copyright 2017 Nidium Inc. All rights reserved.
   Use of this source code is governed by a MIT license
   that can be found in the LICENSE file.
*/


const { setAnimation, Easing } = require("AnimationBlock");


{
    const s_ShadowRoot = require("../Symbols.js").ElementShadowRoot;

    Canvas.prototype.addMultiple = function(...canvases) {
        for (let canvas of canvases) {
            try {
                this.add(canvas);
            } catch(e) {}
        }
    }

    /*
        Inject a 'Layout syntax tree' (LST) into the element.
    */
    Canvas.prototype.inject = function(nml) {
        NML.CreateTree(nml, this, this[s_ShadowRoot]);
    }

    /*
        Remove all child nodes
    */
    Canvas.prototype.empty = function() {
        var children = this.getChildren();

        for (let child of children) {
            child.removeFromParent();
        }

        return children;
    }

    /*
        Replace the content of the element with the specified 'LST'
    */
    Canvas.prototype.replaceWith = function(nml) {
        var ret = this.empty();
        this.inject(nml);

        return ret;
    }

    Object.defineProperty(Canvas.prototype, "computed", {
        get() {
            return this.getDimensions()
        }
    })

    Object.defineProperty(Canvas.prototype, "inherit", {
        get: function() {
            if (!this.__inheritProxy) {
                this.__inheritProxy = new Proxy({}, {
                    get: (target, prop, rcv) => {
                        if (prop in target && target[prop] != undefined) {
                            return target[prop];
                        }

                        let parent = this.getParent();
                        if (parent) {
                            return parent.inherit[prop];
                        }
                    }
                });
            }

            return this.__inheritProxy;
        }
    });

    Canvas.prototype.highlight = function() {}

    Canvas.prototype.tagName = "canvas"

}

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

var Elements = {};

class NidiumNode extends Canvas {
    constructor(width, height, attributes = {}) {
        super(width, height);

        this.left = attributes.left || 0;
        this.top = attributes.top || 0;

        this.onload = function() {
            this._ctx = this.getContext("2d");
        }

        this.onpaint = function() {
            this._ctx.save();
            this.paint(this._ctx);
            this._ctx.restore();
        }

        //this.onload = this.onpaint;
        this.onresize = this.onpaint;
    }

    name() {
        return "DefaultNode"
    }

    paint(ctx) {}
}


Elements.UIButton = class extends NidiumNode {
    constructor(width, height, attributes) {
        super(width, height, attributes);

        this.position = "inline";
        this.cursor = "pointer";

        this._label = attributes.label || "Button";

        this.on("mouseup", function(ev) {
            AnimationBlock(300, Easing.Sinusoidal.Out, function(btn) {
                btn.width += 20;
                btn.height += 20;
                /* TODO: stopPropagation doesn't work? */
                ev.stopPropagation();
            }, this);
        });
    }

    set label(value) {
        this._label = value;
        this.requestPaint();
    }

    get label() {
        return this._label;
    }

    name() {
        return "UiButton";
    }

    paint(ctx) {
        ctx.fillStyle = "#aaa";
        ctx.stokeStyke = "#111";

        ctx.fillRect(0, 0, this.width, this.height, 5, 5);
        ctx.strokeRect(0, 0, this.width-0.5, this.height-0.5, 5, 5);

        ctx.fillStyle = "#000";
        ctx.textAlign = "center";

        ctx.fillText(this._label, this.width/2, this.height/2+4);
    }
}

window._onready = function(lst) {
    document.canvas.inject(lst);
}
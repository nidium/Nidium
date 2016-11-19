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

{

    Object.defineProperty(Canvas.prototype, "inherit", {
        get: function() {
            if (!this.__inheritProxy) {
                this.__inheritProxy = new Proxy({}, {
                    get: (target, prop, rcv) => {
                        if (prop in target) {
                            return target[prop];
                        }

                        let parent = this.getParent();
                        if (parent) {
                            return parent.inh[prop];
                        }
                    }
                });
            }

            return this.__inheritProxy;
        }
    });

    Canvas.currentHightLight = null;

    Canvas.prototype.highlight = function(enable = true) {
        var canvas = null;

        if (!this.getParent() || !this.__visible) {
            return false;
        }

        var draw = (canvas) => {
            canvas.clear();
            let ctx = canvas.getContext("2d");
            ctx.fillStyle = "rgba(111, 108, 220, 0.6)";
            ctx.fillRect(0, 0, canvas.width, canvas.height);
        }

        if (Canvas.currentHightLight) {
            canvas = Canvas.currentHightLight;
            //canvas.setSize(this.width, this.height);

            AnimationBlock(200, Easing.Back.Out, (canvas) => {
                canvas.width = this.width;
                canvas.height = this.height;
            },  canvas);

            canvas.on("resize", () => {
                draw(canvas);
            });

        } else {
            canvas = new Canvas(this.width, this.height);
            Canvas.currentHightLight = canvas;

            canvas.on("load", () => {
                draw(canvas);
            });
        }

        document.canvas.add(canvas);
        canvas.position = "absolute";

        AnimationBlock(200, Easing.Back.Out, (canvas) => {
            canvas.left = this.__left;
            canvas.top = this.__top;
        },  canvas);
    }

}

class DebugCanvas extends Canvas {
    constructor(width, height, parent = document.canvas) {
        super(width, height);

        if (parent) {
            parent.add(this);
        }

        this.onload = this.randomPaint;
        this.onresize = this.randomPaint;
        this.m_highlight = false;
        this._pickColor();
        this.cursor = "pointer";

        this.on("mousedown", function(ev) {
            this.highlight();
        }.bind(this));
        this.on("mouseup", function(ev) {
            this.highlight(false);
        }.bind(this));

    }

    _pickColor() {
        var mr = (min=100, max=200) => min + Math.floor(Math.random()*(max-min));
        this.color = `rgba(${mr(200, 250)}, ${mr()}, ${mr(50, 150)}, 0.7)`;
    }

    randomPaint() {
        var ctx = this.getContext("2d");

        ctx.save()

        this.clear();   

        ctx.fillStyle = this.color;
        
        ctx.fillRect(0, 0, this.width, this.height, 7, 25);

        ctx.fillStyle = "#fff";
        ctx.fillText(`${this.id}`, 10, 15);

        if (this.m_highlight) {
            ctx.lineWidth = 2;
            ctx.strokeStyle = `rgba(210, 210, 210, 1)`;
            ctx.strokeRect(0, 0, this.width, this.height, 10, 20);
        }

        ctx.restore();
    }

    place() {
        var parent = this.getParent();
        if (!parent) {
            return;
        }
        this.left = Math.floor(Math.random() * (parent.width - this.width));
        this.top = Math.floor(Math.random() * (parent.height - this.height));

    }

    /*highlight(enable = true) {
        this.m_highlight = enable;
        this.randomPaint();
    }*/
}

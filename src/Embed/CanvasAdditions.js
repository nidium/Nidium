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

class DebugCanvas extends Canvas {
    constructor(width, height, parent = document.canvas) {
        super(width, height);

        if (parent) {
            parent.add(this);
        }

        this.onload = this.randomPaint;
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
        this.color = `rgba(${mr(50, 100)}, ${mr()}, ${mr(200, 250)}, 0.7)`;
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
        this.left = Math.random() * (parent.width - this.width);
        this.top = Math.random() * (parent.height - this.height);

    }

    highlight(enable = true) {
        this.m_highlight = enable;
        this.randomPaint();
    }
}
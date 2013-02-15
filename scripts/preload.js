/* --------------------------------------------------------------------------- *
 * Native Minimal Built'in FrameWork                       (c) 2013 Stight.com * 
 * --------------------------------------------------------------------------- * 
 * Version:     1.0                                                            *
 *                                                                             *
 * Permission is hereby granted, free of charge, to any person obtaining a     *
 * copy of this software and associated documentation files (the "Software"),  *
 * to deal in the Software without restriction, including without limitation   *
 * the rights to use, copy, modify, merge, publish, distribute, sublicense,    *
 * and/or sellcopies of the Software, and to permit persons to whom the        *
 * Software is furnished to do so, subject to the following conditions:        *
 *                                                                             *
 * The above copyright notice and this permission notice shall be included in  *
 * all copies or substantial portions of the Software.                         *
 *                                                                             *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR  *
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,    *
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE *
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER      *
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING     *
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER         *
 * DEALINGS IN THE SOFTWARE.                                                   *
 * --------------------------------------------------------------------------- * 
 */

/*
Les grands orateurs qui dominent les assemblées par l’éclat de leur parole sont,
en général, les hommes politiques les plus médiocres : il ne faut pas les
combattre par des paroles, ils en ont toujours de plus ronflantes que les vôtres
; il faut opposer à leur faconde un raisonnement serré, logique ; leur force est
dans le vague, il faut les ramener dans la réalité des faits : la pratique les
tue.
*/

var document = {
    body: Native.canvas
};

document.createElement = function(type) {
    switch(type) {
        case "canvas":
            return new Canvas(32, 32);
        default:
            break;
    }
}

Canvas.prototype.appendChild = function(node) {
    this.add(node);
}

if ("titleBarColor" in window) {
	window.titleBarColor = "rgba(200, 200, 200, 0.8)";
}

if ("opacity" in window) {
	window.opacity = 0.95;
}

if ("titleBarControlsOffsetY" in window) {
	window.titleBarControlsOffsetX = 0;
	window.titleBarControlsOffsetY = 0;
}

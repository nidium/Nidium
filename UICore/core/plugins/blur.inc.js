/* -------------------------- */
/* Native (@) 2012 Stight.com */
/* -------------------------- */

/* -----------------------------------------------------------------
 * NativeBlur - an ultra fast dirty blur For Canvas                *
 * ----------------------------------------------------------------- 
 * Version: 	1.0
 * Author:		Vincent Fontaine
 * 
 * Permission is hereby granted, free of charge, to any person
 * obtaining a copy of this software and associated documentation
 * files (the "Software"), to deal in the Software without
 * restriction, including without limitation the rights to use,
 * copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following
 * conditions:
 * 
 * The above copyright notice and this permission notice shall be
 * included in all copies or substantial portions of the Software.
 * 
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES
 * OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
 * NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT
 * HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
 * WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
 * OTHER DEALINGS IN THE SOFTWARE.
 */

canvas.implement({
	blur : function(wx, wy, w, h, pass){
		var r = 1,
			offscreen = new Canvas(w, h);

		offscreen.putImageData(this.getImageData(wx, wy, w, h), 0, 0);

		for(var i=0, g = 0.325; i<pass; i++){
			for (var cy=-r; cy<(r+1); cy++){
				for (var cx=-r; cx<(r+1); cx++){
					offscreen.globalAlpha = g;
					offscreen.drawImage(offscreen, cx-0.15, cy+0.28);
					g = g * 0.98;
				}
			}
			offscreen.globalAlpha = 1;
			offscreen.fillStyle = "rgba(255, 255, 255, 0.036)";
			offscreen.fillRect(0, 0, w, h);
		}
		var blur = offscreen.getImageData(0, 0, w, h);
		this.putImageData(blur, wx, wy);
	},

	fastblur : function(wx, wy, w, h, pass){
		var r = 1,
			offscreen = new Canvas(w, h);

		offscreen.drawImage(this, wx, wy, w, h, 0, 0, w, h);

		for(var i=0, g = 0.325; i<pass; i++){
			for (var cy=-r; cy<(r+1); cy++){
				for (var cx=-r; cx<(r+1); cx++){
					offscreen.globalAlpha = g;
					offscreen.drawImage(offscreen, cx-0.15, cy+0.28);
					g = g * 0.98;
				}
			}
			offscreen.globalAlpha = 1;
			offscreen.fillStyle = "rgba(255, 255, 255, 0.036)";
			offscreen.fillRect(0, 0, w, h);
		}
		this.drawImage(offscreen, wx, wy);
	}
});
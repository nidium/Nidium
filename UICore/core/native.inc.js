/* -------------------------- */
/* Native (@) 2012 Stight.com */
/* -------------------------- */

/* -----------------------------------------------------------------
 * Native Object Framework                                         * 
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

load('core/helper.inc.js');
load('core/layout/core.js');
load('core/events.inc.js');

load('core/layout/elements/UIView.js');
load('core/layout/elements/UIButton.js');
load('core/layout/elements/UIButtonClose.js');
load('core/layout/elements/UIButtonDown.js');

load('core/layout/elements/UIText.js');
load('core/layout/elements/UITextInput.js');

load('core/layout/elements/UIScrollBars.js');
load('core/layout/elements/UIRadio.js');
load('core/layout/elements/UILines.js');

load('core/layout/elements/UITab.js');
load('core/layout/elements/UITabController.js');

load('core/layout/elements/UIDropDownOption.js');
load('core/layout/elements/UIDropDownController.js');

load('core/layout/elements/UIWindow.js');
load('core/layout/elements/UIWindowResizer.js');

load('core/layout/elements/UISliderKnob.js');
load('core/layout/elements/UISliderController.js');

load('core/plugins/blur.inc.js');
load('core/plugins/roundbox.inc.js');
load('core/plugins/tabbox.inc.js');
load('core/plugins/animations.inc.js');

var NativeRenderer = {
	objID : 0,
	focusObj : 0,
	nbObj : 0,

	nodes : {},
	elements : [],

	rootElement : null,

	higherzIndex : 0,

	pasteBuffer : '',

	register : function(element){
		this.nodes[element._uid] = element;
	},

	remove : function(element){
		delete(this.nodes[element._uid]);
		delete(element);
		this.draw();
	},

	clear : function(){
		canvas.clearRect(0, 0, window.width, window.height);
	},

	draw : function(){
		var z = this.getElements();

		if (canvas.__mustBeDrawn || true) {
			for (var i=0; i<z.length; i++){
				z[i].refresh();
				if (z[i].visible){
					z[i].beforeDraw();
					z[i].draw();
					z[i].afterDraw();
				}
			}
			canvas.__mustBeDrawn = false;
		}

	},

	grid : function(){
		for (var x=0; x<canvas.width ; x+=80) {
			canvas.beginPath();
			canvas.moveTo(x, 0);
			canvas.lineTo(x, canvas.height);
			canvas.moveTo(0, x);
			canvas.lineTo(canvas.width, x);
			canvas.stroke();

		}
	},

	_animateFocus : function(element){
		if (element.flags._canReceiveFocus) {
			element.hasFocus = true;
			this.focusObj = element._nid;
		} else {
			this.focusNextElement();
		}
	},

	getElements : function(){
		var elements = [],
			self = this,
			z = 0;

		var dx = function(nodes, parent){
			for (var child in nodes){
				elements.push(nodes[child]);

				nodes[child]._nid = z;
				nodes[child].hasFocus = false;
				if (self.focusObj == z++) {
					self._animateFocus(nodes[child]);
				}
				if (count(nodes[child].nodes)>0) {
					dx(nodes[child].nodes, nodes[child].parent);
				}
			}
		};

		dx(this.nodes, null);
		this.nbObj = z;

		this.elements = elements.sort(function(a, b){
			return a._zIndex - b._zIndex;
		});

		this.higherzIndex = elements[elements.length-1] ? elements[elements.length-1]._zIndex : 0;
		return elements;
	},

	find : function(property, value){
		var elements = [];

		var dx = function(nodes, parent){
			for (var child in nodes){
				if (nodes[child][property] && nodes[child][property] == value){
					elements.push(nodes[child]);
				}
				if (count(nodes[child].nodes)>0) {
					dx(nodes[child].nodes, nodes[child].parent);
				}
			}
		};

		dx(this.nodes, null);

		elements.each = function(cb){
			for (var i in elements) {
				if (elements.hasOwnProperty(i) && elements[i]._uid){
					cb.call(elements[i]);
				}
			}
		};

		return elements;
	},

	getHigherZindex : function(){
		var zindexes = [];

		var dx = function(nodes, parent){
			for (var child in nodes){
				zindexes.push(nodes[child]._zIndex);
				if (count(nodes[child].nodes)>0) {
					dx(nodes[child].nodes, nodes[child].parent);
				}
			}
		};
		dx(this.nodes, null);
		return zindexes.length ? Math.max.apply(null, zindexes) : 0;
	},

	focusNextElement : function(){
		this.focusObj++;
		if (this.focusObj > this.nbObj-2) {
			this.focusObj = 0;
		}
	},

	refresh : function(){
		/* dummy */
	}

};

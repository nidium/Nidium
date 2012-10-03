/* -------------------------- */
/* Native (@) 2012 Stight.com */
/* -------------------------- */

/* -----------------------------------------------------------------------------
 * Native Object Framework (@) 2012 Stight.com                                 * 
 * ----------------------------------------------------------------------------- 
 * Version: 	1.0
 * Author:		Vincent Fontaine
 * 
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 * 
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 * 
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS
 * IN THE SOFTWARE.
 *
 */

var __PATH_CORE__ = 'core/',
	__PATH_LAYOUT__ = 'core/layout/',
	__PATH_ELEMENTS__ = 'core/layout/elements/',
	__PATH_PLUGINS__ = 'core/plugins/';

/* Todo : add more performance flags */
var	__ENABLE_TEXT_SHADOWS__ = false,
	__ENABLE_BUTTON_SHADOWS__ = true,
	__ENABLE_GRADIENT_LAYERS__ = true;

load(__PATH_CORE__ +	'helper.inc.js');
load(__PATH_LAYOUT__ +	'core.js');
load(__PATH_CORE__ +	'events.inc.js');

/* -- Load UI Elements -- */
load(__PATH_ELEMENTS__ + 'UIView.js');
load(__PATH_ELEMENTS__ + 'UIButton.js');
load(__PATH_ELEMENTS__ + 'UIButtonClose.js');
load(__PATH_ELEMENTS__ + 'UIButtonDown.js');

load(__PATH_ELEMENTS__ + 'UIText.js');
load(__PATH_ELEMENTS__ + 'UITextInput.js');

load(__PATH_ELEMENTS__ + 'UIScrollBars.js');
load(__PATH_ELEMENTS__ + 'UIRadio.js');
load(__PATH_ELEMENTS__ + 'UILines.js');

load(__PATH_ELEMENTS__ + 'UITab.js');
load(__PATH_ELEMENTS__ + 'UITabController.js');

load(__PATH_ELEMENTS__ + 'UIDropDownOption.js');
load(__PATH_ELEMENTS__ + 'UIDropDownController.js');

load(__PATH_ELEMENTS__ + 'UIWindow.js');
load(__PATH_ELEMENTS__ + 'UIWindowResizer.js');

load(__PATH_ELEMENTS__ + 'UISliderKnob.js');
load(__PATH_ELEMENTS__ + 'UISliderController.js');

/* -- Load plugins -- */
load(__PATH_PLUGINS__ + 'blur.inc.js');
load(__PATH_PLUGINS__ + 'roundbox.inc.js');
load(__PATH_PLUGINS__ + 'tabbox.inc.js');
load(__PATH_PLUGINS__ + 'animations.inc.js');

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
				if (z[i].isVisible()){
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

		this.higherzIndex = elements[elements.length-1] ?
			elements[elements.length-1]._zIndex : 0;

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

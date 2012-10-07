/* -------------------------- */
/* Native (@) 2012 Stight.com */
/* -------------------------- */

/* -----------------------------------------------------------------------------
 * NOM : Native Object Model Framework (@) 2012 Stight.com                     * 
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

load(__PATH_ELEMENTS__ + 'UILabel.js');
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

load(__PATH_ELEMENTS__ + 'UIDiagram.js');

/* -- Load plugins -- */
load(__PATH_PLUGINS__ + 'blur.inc.js');
load(__PATH_PLUGINS__ + 'spline.inc.js');
load(__PATH_PLUGINS__ + 'roundbox.inc.js');
load(__PATH_PLUGINS__ + 'tabbox.inc.js');
load(__PATH_PLUGINS__ + 'animations.inc.js');

Native.layout = {
	objID : 0,
	focusObj : 0,
	nbObj : 0,

	nodes : {}, // May content several trees of elements 
	elements : [], // flat representation of Trees (zIndex sorted elements)

	rootElement : null,
	currentFocusedElement : null,
	currentOnTopElement : null,

	higherzIndex : 0,

	pasteBuffer : '',

	register : function(rootElement){
		this.nodes[rootElement._uid] = rootElement;
	},

	unregister : function(rootElement){
		delete(this.nodes[rootElement._uid]);
		delete(rootElement);
	},

	destroy : function(element){
		if (element.parent){
			delete(element.parent.nodes[element._uid]);
			delete(this.nodes[element._uid]);
			delete(element);
			element = null;
		}
	},

	collectGarbage : function(z){
		for (var i=z.length-1; i>0; i--){
			z[i].__garbageCollector && this.destroy(z[i]);
		}
	},

	remove : function(rootElement){
		var self = this,
			elements = [];

		this.bubble(rootElement, function(){
			elements.push(this);
			this.__garbageCollector = true;
		});

		this.collectGarbage(elements);
		this.destroy(rootElement);
		canvas.__mustBeDrawn = true;
	},

	/*
	 * Apply Recursive Inference to rootElement and children
	 */
	bubble : function(rootElement, inference){
		var self = this,
			fn = OptionalCallback(inference, null),
			dx = function(z, parent){
				for (var i in z){
					fn.call(z[i]);
					if (z[i] && self.count(z[i].nodes)>0) {
						/* test z[i] as it may be destroyed by inference */
						dx(z[i].nodes, z[i].parent);
					}
				}
			};

		if (rootElement && fn){
			if (rootElement != this) fn.call(rootElement);
			dx(rootElement.nodes);
		}
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

	getElements : function(){
		var elements = [],
			self = this,
			n = 0;

		this.bubble(this, function(){
			elements.push(this);
			this._nid = n;
			this.hasFocus = false;
			if (self.focusObj == n++){
				self._animateFocus(this);
			}
		});

		this.nbObj = n;

		this.elements = elements.sort(function(a, b){
			return a._zIndex - b._zIndex;
		});

		this.higherzIndex = elements[elements.length-1] ?
			elements[elements.length-1]._zIndex : 0;

		return elements;
	},

	find : function(property, value){
		var elements = [],
			self = this;

		this.bubble(this, function(){
			if (this[property] && this[property] == value){
				elements.push(this);
			}
		});

		elements.each = function(cb){
			for (var i in elements) {
				if (elements.hasOwnProperty(i) && elements[i]._uid){
					cb.call(elements[i]);
				}
			}
		};

		return elements;
	},

	count : function(nodes){
		var len = 0;
		for (var i in nodes){
			if (nodes.hasOwnProperty(i)){
				len++;
			}
		}
		return len;
	},

	getHigherZindex : function(){
		var self = this,
			zindexes = [];

		this.bubble(this, function(){
			zindexes.push(this._zIndex);
		});

		return zindexes.length ? Math.max.apply(null, zindexes) : 0;
	},

	focusNextElement : function(){
		this.focusObj++;
		if (this.focusObj > this.nbObj-2) {
			this.focusObj = 0;
		}
	},

	_animateFocus : function(element){
		if (element.flags._canReceiveFocus) {
			this.focus(element);
		} else {
			this.focusNextElement();
		}
	},

	focus : function(element){
		if (element.hasFocus === true) {
			return false;
		}

		if (element.flags._canReceiveFocus) {
			/* Blur last focused element */
			if (this.currentFocusedElement) {
				this.currentFocusedElement.fireEvent("blur", {});
			}

			/* set this element as the new focused element */
			element.hasFocus = true;
			element.fireEvent("focus", {});
			this.currentFocusedElement = element;
			this.focusObj = element._nid;
		}
	},

	bringToTop : function(element){
		if (element.isOnTop === true) {
			return false;
		}

		if (this.currentOnTopElement) {
			this.currentOnTopElement.isOnTop = false;
			this.currentOnTopElement.fireEvent("back", {});
		}

		/* set this element as the new on top element */
		element.isOnTop = true;
		element.fireEvent("top", {});
		element.zIndex = this.getHigherZindex() + 1;
		this.currentOnTopElement = element;
	},

	refresh : function(){
		/* dummy */
	}

};

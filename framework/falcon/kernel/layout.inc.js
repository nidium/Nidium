/* -----------------------------------------------------------------------------
 * Native Object Model Framework (@) 2013 Stight.com                           * 
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

"use strict";

var DOMElement = function(type, options, parent){
	var o = this.options = options || {},
		p = parent ? parent : null; // parent element

	this.parent = p;
	this.nodes = {}; // children elements
	this.type = OptionalString(type, "UIView");

	this.name = OptionalString(o.name, "");
	this.text = OptionalString(o.text, "");
	this.label = OptionalString(o.label, "");

	this._eventQueues = [];
	this._mutex = [];

	this._uid = "_obj_" + Native.layout.objID++;
	this.id = OptionalString(o.id, this._uid);

	// -- coordinate properties
	this._x = OptionalNumber(o.left, 0);
	this._y = OptionalNumber(o.top, 0);
	this._w = o.width ? Number(o.width) : p ? p._w : window.width;
	this._h = o.height ? Number(o.height) : p ? p._h : window.height;

	this.refresh();
};

DOMElement.prototype = {
	__noSuchMethod__ : function(id, args){
		throw("Undefined method " + id);
	},

	add : function(type, options){
		var element = new DOMElement(type, options, this);
		this.addChild(element);
		return element;
	},

	remove : function(){
		Native.layout.remove(this);
	},

	addChild : function(element){
		element.parent = this;
		element.layer = new Canvas(element._w, element._h);
		element.layer.context = element.layer.getContext("2D");
		this.layer.add(element.layer);

		Native.elements.init(element);
		this.nodes[element._uid] = element;
	},

	removeChild : function(element){
		if (element.parent != this) {
			throw("Unable to remove this element.");
		}
		Native.layout.remove(element);
	},

	refresh : function(){
		var p = this.parent;

		this._absx = p ? p._absx + this._x : this._x;
		this._absy = p ? p._absy + this._y : this._y;

		this.__minx = this._absx;
		this.__miny = this._absy;
		this.__maxx = this._absx + this._w;
		this.__maxy = this._absy + this._h;

		this.__needRedraw = true;
	},

	beforeDraw : function(){
		this.layer.left = this._x;
		this.layer.top = this._y;
	},

	draw : function(){

	},

	afterDraw : function(){

	},

	isPointInside : function(mx, my){
		var x1 = this.__minx,
			y1 = this.__miny,
			x2 = this.__maxx,
			y2 = this.__maxy;

		return (mx>=x1 && mx<x2 && my>=y1 && my<y2) ? true : false;
	},

	isVisible : function(){
		return true;
	},

	/* -- Getters And Setters ------------ */

	get left() {
  		return this._x;
	},

	set left(value) {
		var dx = Number(value) - this._x;
		if (dx==0) { return false; }
		this._x += dx;
		this._absx += dx;
		this.refresh();
	},

	get top() {
  		return this._y;
	},

	set top(value) {
		var dy = Number(value) - this._y;
		if (dy==0) { return false; }
		this._y += dy;
		this._absy += dy;
		this.refresh();
	}

};

DOMElement.implement = function(props){
	for (var key in props){
		if (props.hasOwnProperty(key)){
			DOMElement.prototype[key] = props[key];
		}
	}
};

var Application = function(options){
	options = options || {};
	options.x = 0;
	options.y = 0;
	options.w = window.width;
	options.h = window.height;
	options.background = OptionalValue(options.background, '#262722');

	var element = new DOMElement("UIView", options, null);

	element.layer = Native.canvas;
	element.layer.context = Native.canvas.context;

	Native.elements.init(element);
	Native.layout.register(element);

	window.requestAnimationFrame(function(){
//		FPS.start();
		Native.layout.draw();
// 		FPS.show();
	});

	return element;
};


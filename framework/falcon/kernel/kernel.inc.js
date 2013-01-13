/* ------------------------+------------- */
/* Native Framework 2.0    | Falcon Build */
/* ------------------------+------------- */
/* (c) 2013 Stight.com - Vincent Fontaine */
/* -------------------------------------- */

"use strict";

/* -------------------------------------------------------------------------- */

Native.object = {
	__noSuchMethod__ : function(id, args){
		throw "Undefined method " + id;
	},

	__lock : function __lock(method){
		print("__lock" + (method?'('+method+')':'()'), this);
		this._locked = true;
	},

	__unlock : function __unlock(method){
		print("__unlock" + (method?'('+method+')':'()'), this);
		this._locked = false;
	},

	refresh : function refresh(){
		print("refresh()", this);

		this.layer.visible = this._visible;
		this.updateInheritance();

		if (this._needOpacityUpdate) this.updateLayerOpacity();
		if (this._needPositionUpdate) this.updateLayerPosition();
		if (this._needSizeUpdate) this.updateLayerSize();
		if (this._needAncestorCacheClear) this.updateAncestors();
		if (this._needRedraw) this.redraw();

		this._needRefresh = false;
	},

	updateInheritance : function updateInheritance(){
		var p = this.parent,
			x = this._left + this._offsetLeft,
			y = this._top + this._offsetTop;

		this.__scrollLeft = p ? p.__scrollLeft + this._scrollLeft 
							 : this._scrollLeft;

		this.__scrollTop = p ? p.__scrollTop + this._scrollTop 
							 : this._scrollTop;

		this.__opacity = p ? p.__opacity * this._opacity : this._opacity;

		this.__overflow = p ? p.__overflow && this._overflow : this._overflow;
		this.__fixed = p ? p.__fixed || this._fixed : this._fixed;

		this.__layerPadding = p ? p._layerPadding - this._layerPadding 
								: this._layerPadding;

		var	sx = (this.__fixed===false ?
				 (p ? p._scrollLeft : this._scrollLeft) : this._scrollLeft),

			sy = (this.__fixed===false ?
				 (p ? p._scrollTop : this._scrollTop) : this._scrollTop);

		this.__left = (p ? p.__left + x : x) - sx;
		this.__top = (p ? p.__top + y : y) - sy;
/*
		this.__left = this.layer.__left + this._layerPadding;
		this.__top = this.layer.__top + this._layerPadding;
*/

	},

	updateLayerOpacity : function updateLayerOpacity(){
		print("updateLayerOpacity()", this);
		this.layer.context.globalAlpha = this.__opacity;
		this._needOpacityUpdate = false;
	},

	updateLayerPosition : function updateLayerPosition(){
		var p = this.parent,
			sx = (this.__fixed===false ? (p ? p._scrollLeft : this._scrollLeft) : this._scrollLeft),
			sy = (this.__fixed===false ? (p ? p._scrollTop : this._scrollTop) : this._scrollTop);

		print("updateLayerPosition()", this);
		this.layer.left = this._left + this.__layerPadding - sx;
		this.layer.top = this._top + this.__layerPadding - sy;
		this._needPositionUpdate = false;
	},

	updateLayerSize : function updateLayerSize(){
		print("updateLayerSize()", this);
		this.layer.width = this.getLayerPixelWidth();
		this.layer.height = this.getLayerPixelHeight();
		this._needSizeUpdate = false;
	},

	updateAncestors : function updateAncestors(){
		print("updateAncestors()", this);
		var element = this;

		/* clean cache for all this element's parents (all ancestors) */
		while (element.parent){
			var p = element.parent;
			p._cachedContentWidth = null;
			p._cachedContentHeight = null;

			/* refresh ancestor's scrollbars */
			if (p.loaded && p.type=="UIView" && p.overflow===false)
				p.refreshScrollBars();

			element = p;
		}
		this._needAncestorCacheClear = false;
	},

	redraw : function redraw(){
		print("redraw()", this)
		this.layer.clear();
		if (this.layer.debug) this.layer.debug();

		this.beforeDraw(this.layer.context);
		this.draw(this.layer.context);
		this.afterDraw(this.layer.context);

		this._needRedraw = false;
	},

	add : function add(type, options){
		print("add()", this);
		var element = new DOMElement(type, options, this);
		this.addChild(element);
		return element;
	},

	remove : function remove(){
		Native.layout.remove(this);
		Native.layout.update();
	},

	show : function show(){
		this.visible = true;
		return this;
	},

	hide : function hide(){
		this.visible = false;
		return this;
	},

	focus : function focus(){
		return this;
	},

	addChild : function addChild(element){
		if (this.nodes[element._uid] || !isDOMElement(element)) return false;
		print("addChild("+element._uid+")", this);
		this.nodes[element._uid] = element;
		element._root = this._root;
		element.parent = this;
		element.parent.layer.add(element.layer);
		Native.layout.update();
	},

	removeChild : function removeChild(element){
		if (element.parent && element.parent != this){
			throw("Unable to remove this element.");
		}
		Native.layout.remove(element);
		Native.layout.update();
	},

	getLayerPixelWidth : function getLayerPixelWidth(){
		return Math.round(this._width + 2*this._layerPadding);
	},

	getLayerPixelHeight : function getLayerPixelHeight(){
		return Math.round(this._height + 2*this._layerPadding);
	},

	/*
	 * Sort DOMElements to match hardware physical layers order.
	 */
	resetNodes : function resetNodes(){
		if (!this.parent) return false;

		print("resetNodes()", this);

		var parent = this.parent, // parent of this virtual element
			layers = parent.layer.getChildren(); // physical children

		/* Reset parent's nodes */
		parent.nodes = {};

		/* reconstruct the nodes in the right order */
		for (var i in layers){
			// get the host element (that's it : the virtual element)
			var element = layers[i].host;

			// add the element in parent's nodes 
			parent.nodes[element._uid] = element;
		}

		Native.layout.update();
	},

	bringToFront : function bringToFront(){
		this.layer.bringToFront();
		this.resetNodes();
		return this;
	},

	sendToBack : function sendToBack(){
		this.layer.sendToBack();
		this.resetNodes();
		return this;
	},

	getDrawingBounds : function getDrawingBounds(){
		var p = this.parent;
		return {
			x : 0 + this._offsetLeft + this._layerPadding,
			y : 0 + this._offsetTop + this._layerPadding,
			w : this._width,
			h : this._height,
			textOffsetX : 0,
			textOffsetY : 0
		};
	},

	beforeDraw : function beforeDraw(){
		print("beforeDraw()", this);
	},

	afterDraw : function afterDraw(){
		print("afterDraw()", this);
	},

	isPointInside : function isPointInside(mx, my){
		var x1 = this.__left+1,
			y1 = this.__top+2,
			x2 = x1 + this._width,
			y2 = y1 + this._height;

		return (mx>=x1 && mx<x2 && my>=y1 && my<y2) ? true : false;
	},

	isVisible : function isVisible(){
		return this._visible;
	},

	hasClass : function hasClass(name){
		return new RegExp('(\\s|^)'+name+'(\\s|$)').test(this.className);
	},

	addClass : function addClass(name){
		if (!this.hasClass(name)){
			this.className += (this.className ? ' ' : '') + name;
		}
		return this;
	},

	setClass : function setClass(name){
		this.className = name;
		return this;
	},

	removeClass : function removeClass(name){
		if (this.hasClass(name)){
			let r = new RegExp('(\\s|^)'+name+'(\\s|$)'),
				k = this.className;

			this.className = k.replace(r,' ').replace(/^\s+|\s+$/g, '');
		}
		return this;
	},

	updateProperties : function updateProperties(){
		print("updateProperties()", this);
		var classNames = this.className.split(" ");

		for (var i in classNames){
			var props = Native.StyleSheet.getProperties(classNames[i]);

			this.setProperties(props);
		}
	},

	setProperties : function setProperties(options){
		for (var key in options){
			if (options.hasOwnProperty(key)){
				this[key] = options[key];
			}
		}
		return this;
	},

	toString : function toString(){
		return "Object Manager";
	}
};

/* -------------------------------------------------------------------------- */


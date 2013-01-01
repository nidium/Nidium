/* ------------------------+------------- */
/* Native Framework 2.0    | Falcon Build */
/* ------------------------+------------- */
/* (c) 2013 Stight.com - Vincent Fontaine */
/* -------------------------------------- */

"use strict";

/* -------------------------------------------------------------------------- */

Native.object = {
	__noSuchMethod__ : function(id, args){
		throw("Undefined method " + id);
	},

	__lock : function(){
		this._locked = true;
	},

	__unlock : function(){
		this._locked = false;
	},

	refresh : function(){
		var p = this.parent,
			x = this._left + this._offsetLeft,
			y = this._top + this._offsetTop;

		/*
		this._absx = p ? p._absx + x : x;
		this._absy = p ? p._absy + y : y;
		*/

		this.__layerPadding = p ? p._layerPadding - this._layerPadding 
								: this._layerPadding;

		this.layer.visible = this._visible;
		
		if (this._needOpacityUpdate){
			this.layer.context.globalAlpha = this._opacity;
			this._needOpacityUpdate = false;
		}

		if (this._needPositionUpdate){
			this.layer.left = Math.round(this._left + this.__layerPadding);
			this.layer.top = Math.round(this._top + this.__layerPadding);
			this._needPositionUpdate = false;
		}

		if (this._needSizeUpdate){
			var w = this.getLayerPixelWidth(),
				h = this.getLayerPixelHeight();
			this.layer.width = w;
			this.layer.height = h;
			this._needSizeUpdate = false;
		}

		if (this._needRedraw) {
			this.layer.clear();
			if (this.layer.debug) this.layer.debug();
			this.draw(this.layer.context);
			this._needRedraw = false;
		}

		this._needRefresh = false;
	},

	add : function(type, options){
		var element = new DOMElement(type, options, this);
		this.addChild(element);
		return element;
	},

	remove : function(){
		Native.layout.remove(this);
		Native.layout.update();
	},

	show : function(){
		if (!this.visible) {
			this.visible = true;
		}
		return this;
	},

	hide : function(){
		if (this.visible) {
			this.visible = false;
		}
		return this;
	},

	focus : function(){
		return this;
	},

	addChild : function(element){
		this.nodes[element._uid] = element;
		element.parent = this;
		element.parent.layer.add(element.layer);
		Native.layout.update();
	},

	removeChild : function(element){
		if (element.parent != this) {
			throw("Unable to remove this element.");
		}
		Native.layout.remove(element);
		Native.layout.update();
	},

	getLayerPixelWidth : function(){
		return Math.round(this._width + 2*this._layerPadding);
	},

	getLayerPixelHeight : function(){
		return Math.round(this._height + 2*this._layerPadding);
	},

	/*
	 * Sort DOMElements to match hardware physical layers order.
	 */
	resetNodes : function(){
		if (!this.parent) return false;

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

	bringToFront : function(){
		this.layer.bringToFront();
		this.resetNodes();
		return this;
	},

	sendToBack : function(){
		this.layer.sendToBack();
		this.resetNodes();
		return this;
	},

	getDrawingBounds : function(){
		var p = this.parent;
		return {
			x : 0 + this.offsetLeft + this._layerPadding,
			y : 0 + this.offsetTop + this._layerPadding,
			w : this.width,
			h : this.height
		};
	},

	beforeDraw : function(){

	},

	afterDraw : function(){

	},

	isPointInside : function(mx, my){
		this._absx = Math.round(this.layer.__left + this._layerPadding);
		this._absy = Math.round(this.layer.__top + this._layerPadding);

		var x1 = this._absx+1,
			y1 = this._absy+2,
			x2 = x1 + this._width,
			y2 = y1 + this._height;

		return (mx>=x1 && mx<x2 && my>=y1 && my<y2) ? true : false;
	},

	isVisible : function(){
		return this.visible;
	},

	hasClass : function(name){
		return new RegExp('(\\s|^)'+name+'(\\s|$)').test(this.className);
	},

	addClass : function(name){
		if (!this.hasClass(name)){
			this.className += (this.className ? ' ' : '') + name;
		}
		return this;
	},

	setClass : function(name){
		this.className = name;
		return this;
	},

	removeClass : function(name){
		if (this.hasClass(name)){
			let r = new RegExp('(\\s|^)'+name+'(\\s|$)'),
				k = this.className;

			this.className = k.replace(r,' ').replace(/^\s+|\s+$/g, '');
		}
		return this;
	},

	setProperties : function(options){
		for (var key in options){
			if (options.hasOwnProperty(key)){
				this[key] = options[key];
			}
		}
		return this;
	}
};

/* -------------------------------------------------------------------------- */

Native.StyleSheet = {
	document : {},
	add : function(sheet){
		for (var k in sheet){
			if (sheet.hasOwnProperty(k)){
				if (this.document[k]){
					this.mergeProperties(k, sheet[k]);
				} else {
					this.document[k] = sheet[k];
				}
			}
		}
	},

	set : function(element){
		var k = element.className.split(" ");

		for (var i in k){
			var klass = k[i],
				props = this.getProperties(klass);

			for (var key in props){
				if (props.hasOwnProperty(key)) {
					element[key] = props[key];
				}
			}
		}
	},

	mergeProperties : function(klass, properties){
		var prop = this.document[klass];
		for (var p in properties){
			prop[p] = properties[p];
		}
	},

	getProperties : function(klass){
		return this.document[klass];
	}
};

/* -------------------------------------------------------------------------- */


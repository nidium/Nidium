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

		if (this._needOpacityUpdate) this.updateLayerOpacity();
		if (this._needPositionUpdate) this.updateLayerPosition();
		if (this._needSizeUpdate) this.updateLayerSize();
		if (this._needAncestorCacheClear) this.updateAncestors();
		if (this._needRedraw) this.redraw();

		this.layer.overflow = this._overflow;
		this.layer.position = this._position;
		this.layer.visible = this._visible;

		this._needRefresh = false;
	},

	updateAncestors : function updateAncestors(){
		var element = this;
		if (this.layer.__fixed) return false;
		print("updateAncestors("+this._left+", "+this._top+", "+this._width+", "+this._height+")", this);

		/* clean cache for all this element's parents (all ancestors) */
		while (element.parent){
			var p = element.parent;
			if (p.scrollbars) p.refreshScrollBars();
			element = p;
		}
		this._needAncestorCacheClear = false;
	},

	updateLayerOpacity : function updateLayerOpacity(){
		print("updateLayerOpacity()", this);
		this.layer.opacity = this._opacity;
		this._needOpacityUpdate = false;
	},

	updateLayerPosition : function updateLayerPosition(){
		print("updateLayerPosition("+(this._left)+", "+(this._top)+")", this);
		this.layer.left = this._left;
		this.layer.top = this._top;
		this.layer.scrollTop = this._scrollTop;
		this.layer.scrollLeft = this._scrollLeft;
		this.__left = this.layer.__left;
		this.__top = this.layer.__top;
		this._needPositionUpdate = false;
	},

	updateLayerSize : function updateLayerSize(){
		print("updateLayerSize("+this._width+", "+this._height+")", this);
		this.layer.width = Math.round(this._width);
		this.layer.height = Math.round(this._height);
		this._needSizeUpdate = false;
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
		print("addChild("+element._uid+")" + " ("+element.left+", "+element.top+", "+element.width+", "+element.height+")", this);
		this.nodes[element._uid] = element;
		element.parent = this;
		element.parent.layer.add(element.layer);
		element.updateAncestors();
		Native.layout.update();
	},

	removeChild : function removeChild(element){
		if (element.parent && element.parent != this){
			throw("Unable to remove this element.");
		}
		Native.layout.remove(element);
		Native.layout.update();
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
			x : 0 + this._offsetLeft,
			y : 0 + this._offsetTop,
			w : this._width,
			h : this._height,
			textOffsetX : 0,
			textOffsetY : 0
		};
	},

	beforeDraw : function beforeDraw(){
		print("beforeDraw()", this);
		var ctx = this.layer.context,
			rad = this.angle * (Math.PI/180);

		ctx.save();
		ctx.globalAlpha = this._alpha;
		ctx.translate(this._width/2, this._height/2);
		ctx.rotate(rad);
		ctx.translate(-this._width/2, -this._height/2);
	},

	afterDraw : function afterDraw(){
		print("afterDraw()", this);
		var ctx = this.layer.context;
		ctx.restore();
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


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
		return this;
	},

	__unlock : function __unlock(method){
		print("__unlock" + (method?'('+method+')':'()'), this);
		this._locked = false;
		return this;
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
		return this;
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
		return this;
	},

	updateLayerOpacity : function updateLayerOpacity(){
		print("updateLayerOpacity()", this);
		this.layer.opacity = this._opacity;
		this._needOpacityUpdate = false;
		return this;
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
		return this;
	},

	updateLayerSize : function updateLayerSize(){
		print("updateLayerSize("+this._width+", "+this._height+")", this);
		this.layer.width = Math.round(this._width);
		this.layer.height = Math.round(this._height);
		this._needSizeUpdate = false;
		return this;
	},

	redraw : function redraw(){
		print("redraw()", this);
		this.layer.clear();
		if (this.layer.debug) this.layer.debug();
		this.beforeDraw(this.layer.context);
		if (this._visible) this.draw(this.layer.context);
		this.afterDraw(this.layer.context);

		this._needRedraw = false;
		return this;
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

	centerLeft : function(){
		var p = this.parent ? this.parent : document;
		this.left = (p._width - this.width)/2;
		return this;
	},

	centerTop : function(){
		var p = this.parent ? this.parent : document;
		this.top = (p._height - this.height)/2;
		return this;
	},

	center : function(){
		this.centerTop();
		this.centerLeft();
		return this;
	},

	move : function(x, y){
		this.left = this._left + x;
		this.top = this._top + y;
		return this;
	},

	fix : function(){
		this.position = "fixed";
		return this;
	},

	addChild : function addChild(element){
		if (this.nodes[element._uid] || !isDOMElement(element)) return false;
		print("addChild("+element._uid+")" + " ("+element.left+", "+element.top+", "+element.width+", "+element.height+")", this);
		this.nodes[element._uid] = element;
		if (!this.firstChild) this.firstChild = element;
		this.lastChild = element;
		element.parent = this;
		element.parent.layer.add(element.layer);
		element.updateAncestors();
		Native.layout.update();
		return this;
	},

	removeChild : function removeChild(element){
		if (element.parent && element.parent != this){
			throw("Unable to remove this element.");
		}
		Native.layout.remove(element);
		Native.layout.update();
		return this;
	},

	/*
	 * Sort DOMElements to match hardware physical layers order.
	 */
	resetNodes : function resetNodes(){
		if (!this.parent) return false;

		print("resetNodes()", this);

		var parent = this.parent, // parent of this virtual element
			element = null,
			layers = parent.layer.getChildren(); // physical children

		/* Reset parent's nodes */
		parent.nodes = {};

		/* reconstruct the nodes in the right order */
		for (var i in layers){
			// get the host element (that's it : the virtual element)
			element = layers[i].host;

			if (!parent.firstChild) {
				parent.firstChild = element;
			}

			// add the element in parent's nodes 
			parent.nodes[element._uid] = element;
		}
		parent.lastChild = element;


		Native.layout.update();
	},

	getChildren : function(){
		var self = this,
			elements = [];

		var dx = function(z){
			for (var i in z){
				if (isDOMElement(z[i])) elements.push(z[i]);
				dx(z[i].nodes);
			}
		};

		dx(this.nodes);
		return elements;
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

	resizeLayer : function(){
		/* layer rotation realtime padding update hack */
		var b = this.getBoundingRect(),
			w = b.x2 - b.x1,
			h = b.y2 - b.y1;

		this.layer.padding = h/2;
		this.redraw();
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

	getBoundingRect : function(){
		var x1 = this.__left,
			y1 = this.__top,
			x2 = x1 + this.width,
			y2 = y1 + this.height,

			origin = {
				x : this.__left + this._width/2,
				y : this.__top + this._height/2
			};


		if (this.angle % 360 === 0){
			return {
				x1 : x1,
				y1 : y1,
				x2 : x2,
				y2 : y2
			}
		}

		var rad = this.angle * (Math.PI/180),
			r = Math.rotate,
			
			tl = r(x1, y1, origin.x, origin.y, rad),
			tr = r(x2, y1, origin.x, origin.y, rad),
			br = r(x2, y2, origin.x, origin.y, rad),
			bl = r(x1, y2, origin.x, origin.y, rad);

		return {
			x1 : Math.min(tl.x, tr.x, br.x, bl.x),
			y1 : Math.min(tl.y, tr.y, br.y, bl.y),
			x2 : Math.max(tl.x, tr.x, br.x, bl.x),
			y2 : Math.max(tl.y, tr.y, br.y, bl.y)
		}
	},

	beforeDraw : function beforeDraw(){
		print("beforeDraw()", this);
		var ctx = this.layer.context,
			rad = this.angle * (Math.PI/180),
			origin = {
				x : this._width/2,
				y : this._height/2
			};

		ctx.save();
		ctx.globalAlpha = this._alpha;
		ctx.translate(origin.x, origin.y);
		ctx.rotate(rad);
		ctx.translate(-origin.x, -origin.y);
	},

	afterDraw : function afterDraw(){
		print("afterDraw()", this);
		var ctx = this.layer.context;
		ctx.restore();
	},

	isPointInside : function isPointInside(mx, my){
		var x1 = this.__left+1,
			y1 = this.__top,
			x2 = x1 + this._width,
			y2 = y1 + this._height;

		if (this.angle != 0){
			var rad = this.angle * (Math.PI/180),
				origin = {
					x : this.__left + this._width/2,
					y : this.__top + this._height/2
				},
				mouse = Math.rotate(mx, my, origin.x, origin.y, rad);

			mx = mouse.x;
			my = mouse.y;
		}

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


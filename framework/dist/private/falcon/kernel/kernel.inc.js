/* ------------------------+------------- */
/* Native Framework 2.0    | Falcon Build */
/* ------------------------+------------- */
/* (c) 2013 nidium.com - Vincent Fontaine */
/* -------------------------------------- */

"use strict";

/* -------------------------------------------------------------------------- */

Native.object = {
	__noSuchMethod__ : function(id, args){
		throw "Undefined method " + id;
	},

	__lock : function __lock(method){
		this._locked = true;
		return this;
	},

	__unlock : function __unlock(method){
		this._locked = false;
		return this;
	},

	refresh : function refresh(){
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

		/* Refresh ancestors' ScrollBars */
		while (element.parent){
			var p = element.parent;
			if (p.scrollable) p.refreshScrollBars();
			element = p;
		}
		this._needAncestorCacheClear = false;
		return this;
	},

	updateLayerOpacity : function updateLayerOpacity(){
		this.layer.opacity = this._opacity;
		this._needOpacityUpdate = false;
		return this;
	},

	updateLayerPosition : function updateLayerPosition(){
		this.layer.left = this._left;
		this.layer.top = this._top;
		this.layer.scrollTop = this._scrollTop;
		this.layer.scrollLeft = this._scrollLeft;
		this._needPositionUpdate = false;
		return this;
	},

	updateLayerSize : function updateLayerSize(){
		this.layer.width = Math.round(this._width);
		this.layer.height = Math.round(this._height);
		this._needSizeUpdate = false;
		return this;
	},

	redraw : function redraw(){
		this.layer.clear();
		if (this.layer.debug) this.layer.debug();

		this.beforeDraw(this.layer.context);
		if (this._visible) this.draw(this.layer.context);
		this.afterDraw(this.layer.context);

		this._needRedraw = false;
		return this;
	},

	add : function add(type, options){
		var element = new NDMElement(type, options, this);
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
		Native.layout.focus(this);
		return this;
	},

	centerLeft : function centerLeft(){
		var p = this.parent ? this.parent : document;
		this.left = (p._width - this.width)/2;
		return this;
	},

	centerTop : function centerTop(){
		var p = this.parent ? this.parent : document;
		this.top = (p._height - this.height)/2;
		return this;
	},

	center : function center(){
		this.centerTop();
		this.centerLeft();
		return this;
	},

	move : function move(x, y){
		this.left = this._left + x;
		this.top = this._top + y;
		return this;
	},

	place : function place(left, top){
		this.left = left;
		this.top = top;
		return this;
	},

	/*
	 *	setCoordinates() is much faster than place()
	 *  as it does not use our internal setter chain
	 */
	setCoordinates : function setCoordinates(x, y){
		this._left = x;
		this._top = y;
		this.layer.left = x;
		this.layer.top = y;
		return this;
	},

	fix : function fix(){
		this.position = "fixed";
		return this;
	},

	addChild : function addChild(element){
		if (!isNDMElement(element)) return false;

		/* fire the onAddChildRequest event */
		if (this.onAddChildRequest.call(this, element) === false) return false;

		this.nodes.push(element);

		if (!this.firstChild) this.firstChild = element;
		this.lastChild = element;

		element._root = this._root;
		element.parent = this;

		Native.layout.init(element);

		/* fire the new element's onAdoption event */
		element.onAdoption.call(element, this);

		/* fire the element's parent onChild event */
		this.onChildReady.call(this, element);

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

	insertChildAtIndex : function insertChildAtIndex(element, index){

	},

	insertBefore : function insertBefore(element, refElement){
		
	},

	insertAfter : function insertAfter(element, refElement){

	},

	clear : function clear(){
		var element = this;
		while (element.firstChild) {
			element.firstChild.remove();
		}
		Native.layout.update();
	},

	/*
	 * Sort NDMElements to match hardware physical layers order.
	 */
	resetNodes : function resetNodes(){
		if (!this.parent) return false;


		var parent = this.parent, // parent of this virtual element
			element = null,
			layers = parent.layer.getChildren(); // physical children

		/* Reset parent's nodes */
		parent.nodes = [];
		parent.firstChild = null;

		/* reconstruct the nodes in the right order */
		for (var i=0; i<layers.length; i++){
			// get the host element (that's it : the virtual element)
			element = layers[i].host;

			if (!parent.firstChild) {
				parent.firstChild = element;
			}

			// add the element in parent's nodes 
			parent.nodes.push(element);
		}
		parent.lastChild = element;


		Native.layout.update();
	},

	getChildren : function getChildren(){
		var self = this,
			elements = [];

		var dx = function(z){
			for (var i=0; i<z.length; i++){
				if (isNDMElement(z[i])) elements.push(z[i]);
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

	resizeLayer : function resizeLayer(){
		/* layer rotation realtime padding update hack */
		var b = this.getBoundingRect(),
			w = b.x2 - b.x1,
			h = b.y2 - b.y1;

		this.layer.padding = h/2;
		this.redraw();
	},

	expand : function expand(width, height){
		var dx = (width - this._width)/2,
			dy = (height - this._height)/2;

		this.width = width;
		this.height = height;
		this.left -= dx;
		this.top -= dy;
		this.childNodes.each(function(){
			this.left += dx;
			this.top += dy;
		});
		return this;
	},

	shrink : function shrink(b){
		var dx = b.left,
			dy = b.top;

		this.left += dx;
		this.top += dy;

		this.width = b.width;
		this.height = b.height;
		this.childNodes.each(function(){
			this.left -= dx;
			this.top -= dy;
		});
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

	getBoundingRect : function getBoundingRect(){
		var r = this.layer.getVisibleRect(),
			x1 = r.left + 1,
			y1 = r.top + 1,
			x2 = x1 + r.width,
			y2 = y1 + r.height,
			
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
		var ctx = this.layer.context;
		ctx.restore();
	},

	isPointInside : function isPointInside(mx, my){
		var r = this.layer.getVisibleRect ? this.layer.getVisibleRect() : {
			left : this.__left + 1,
			top : this.__top,
			width : this._width,
			height : this._height	
		},
			x1 = r.left + 1,
			y1 = r.top + 1,
			x2 = x1 + r.width,
			y2 = y1 + r.height;

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

	isAncestor : function isAncestor(element){
		if (!isNDMElement(element)) return false;
		if (this.ownerDocument != element.ownerDocument) return false;
		for (var e = element; e; e = e.parent) {
			if (e === this) return true;
		}
		return false;
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

	setProperties : function setProperties(properties){
		for (var k in properties){
			if (properties.hasOwnProperty(k)){
				this[k] = properties[k];
			}
		}
		return this;
	},

	isBoundBySelector : function(selector){
		var result = false,
			l = selector.length,
			s = selector.substr(0, 1),
			p = s.in(".", "@", "#", "*") ? selector.substr(-(l-1)) : selector,
			m = p.split(":"),
			k = m[0],
			states = m[1];

		switch (s) {
			case "@" : /* static property container, do nothing */ break;
			case "#" : if (this.id == k) result = true; break;
			case "." : if (this.hasClass(k)) result = true; break;
			default  : if (this.type == k) result = true; break;
		};

		if (result && states) {
			var temp = [],
				st = states.split('+');  // UITextField:hover+disabled

			if (st.length>0) {
				var checked = 0;
				for (var j=0; j<st.length; j++) {
					var state = st[j];

					if (this[state]) checked++;
				}
				result = (checked == st.length) ? true : false;
			}
		}
		return result;
	},

	getPropertyHandler : function(selector, property){
		var that = this;

		return {
			set value(value) {
				that[property] = value;
			},

			get value() {
				return that[property];
			},
		};
	},

	applySelectorProperties : function(selector, properties){
		if (this.isBoundBySelector(selector) === false) return this;

		if (typeof properties == "function") {
			/* handle function assigned to selector */
			properties.call(this);
		} else {
			/* handle property object assigned to selector */
			for (var k in properties) {
				if (properties.hasOwnProperty(k)) {
					var value = properties[k];

					if (typeof value == "function") {
						/* handle function assigned to property */
						var e = this.getPropertyHandler(selector, k);
						var ret = value.call(e);
						if (ret != undefined) this[k] = ret;
					} else {
						/* handle normal assignation */
						this[k] = value;
					}
				}
			}
		}

		return this;
	},

	applyStyleSheet : function applyStyleSheet(){
		var selectors = document.stylesheet;

		for (var k in selectors){
			if (selectors.hasOwnProperty(k)){
				this.applySelectorProperties(k, selectors[k]);
			}
		}
	},

	toString : function toString(){
		return "Object Manager";
	}
};

/* -------------------------------------------------------------------------- */


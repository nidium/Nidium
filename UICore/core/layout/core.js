/* -------------------------- */
/* Native (@) 2012 Stight.com */
/* -------------------------- */

"use strict";

var DOMElement = function(type, options, parent){
	this.options = options || {};
	this.parent = parent ? parent : null; // parent element
	this.nodes = {}; // children elements

	if (!Native.elements[type]) {
		throw("Undefined element " + type);
	}

	var element = this,
		o = this.options;

	this._uid = "_obj_" + Native.layout.objID++;
	this.id = OptionalString(o.id, this._uid);

	this.type = OptionalString(type, "UIView");
	this.name = OptionalString(o.name, "");
	this.text = OptionalString(o.text, "");
	this.label = OptionalString(o.label, "");

	this._eventQueues = [];
	this._mutex = [];

	// -- coordinate properties
	this.x = OptionalNumber(o.x, 0);
	this.y = OptionalNumber(o.y, 0);
	this.w = o.w ? o.w : this.parent ? this.parent.w : canvas.width;
	this.h = o.h ? o.h : this.parent ? this.parent.h : canvas.height;

	this.scroll = {
		top : OptionalNumber(o.scrollTop, 0)
	};

	this.rotate = 0;
	this.scale = OptionalNumber(o.scale, 1);
	this.zIndex = OptionalNumber(o.zIndex, 0);
	this.opacity = OptionalNumber(o.opacity, 1);

	// -- misc flag
	this.hover = false;
	this.hasFocus = false;
	this.isOnTop = false;
	this.mouseOverPath = false;
	this.visible = OptionalBoolean(o.visible, true);
	this.overflow = OptionalBoolean(o.overflow, true);
	this.fixed = OptionalBoolean(o.fixed, true);
	this.selected = OptionalBoolean(o.selected, false);
	this.draggable = OptionalBoolean(o.draggable, false);

	// -- style properties
	this.blur = OptionalNumber(o.blur, 0);

	this.backgroundBlur = OptionalNumber(o.backgroundBlur, 0);
	this.background = OptionalValue(o.background, '');

	this.color = OptionalValue(o.color, '');
	this.radius = OptionalNumber(o.radius, 0, 0);
	this.shadowBlur = OptionalNumber(o.shadowBlur, 0);
	this.lineWidth = OptionalNumber(o.lineWidth, 1);
	this.lineHeight = OptionalNumber(o.lineHeight, 18);
	this.fontSize = OptionalNumber(o.fontSize, 12);
	this.fontType = OptionalString(o.fontType, "arial");

	var align = OptionalString(o.textAlign, 'left').toLowerCase();
	this.textAlign = align && (align=="left" || align=="right" ||
					 align=="justify" || align=="center") ? align : 'left';

	this.callback = OptionalCallback(o.callback, null);

	// -- dynamic properties inherits from parent at draw time
	this._rotate = this.rotate;
	this._scale = this.scale;
	this._x = this.x;
	this._y = this.y;
	this._opacity = this.opacity;
	this.scroll._top = this.scroll.top;

	this._rIndex = 0;
	this._zIndex = this._rIndex + this.zIndex;

	this._visible = this.visible;
	this._overflow = this.overflow;
	this._fixed = this.fixed;

	// -- transform matrix inheritance (experimental)
	this.g = {
		x : 0,
		y : 0
	};

	this._g = {
		x : 0,
		y : 0
	};

	this.t = {
		_x : 0,
		_y : 0,
		x : 0,
		y : 0
	};

	// -- misc flags
	this.flags = {
		_canReceiveFocus : false,
		_outlineOnFocus : true
	};

	// -- scroll properties
	this.scroll = {
		top : 0,
		left : 0
	};

	this.content = {
		width : this.w,
		height : this.h
	};

	// -- blur box
	this.blurbox = {
		x : 0,
		y : 0,
		w : 0,
		h : 0
	};

	// -- launch element constructor and init dynamic properties
	this.__construct();
	this.refresh();
};

DOMElement.prototype = {
	__construct : function(){

	},

	__noSuchMethod__ : function(id, args){
		throw("Undefined method " + id);
	},

	add : function(type, options){
		var element = new DOMElement(type, options, this);

		Native.elements.init(element);
		this.addChild(element);
		return element;

		/*
		element.__defineGetter__("nbnodes", function() {
			return Native.layout.count(element.nodes);
		});
		*/
	},

	clone : function(){
		var element = new DOMElement(this.type, this.options, this.parent);
		for (var i in this){
			element[i] = this[i];
		}

		element._uid = this._uid + "_clone";
		element.id = element._uid;

		element._rIndex = Native.layout.getHigherZindex() + 1;
		element.opacity = 0.8;
		element.nodes = {}; // kill children nodes

		Native.elements.init(element);
		Native.layout.register(element);

		return element;
	},

	addChild : function(element){
		this.nodes[element._uid] = element;
	},

	removeChild : function(element){
		if (element.parent != this) {
			throw("Unable to remove this element.");
		}
		Native.layout.remove(element);
	},

	remove : function(){
		Native.layout.remove(this);
	},

	show : function(){
		if (!this.visible) {
			this.visible = true;
			Native.layout.refresh();
		}
	},

	hide : function(){
		if (this.visible) {
			this.visible = false;
			Native.layout.refresh();
		}
	},

	focus : function(){
		Native.layout.focus(this);
	},

	bringToTop : function(){
		Native.layout.bringToTop(this);
	},

	refresh : function(){
		// -- dynamic properties
		// -- properties prefixed with _ inherits from parent at draw time
		var p = this.parent;


		this.scroll._top = p ? p.scroll._top + this.scroll.top : 
							   this.scroll.top;

		this._x = p ? p._x + this.x : this.x;
		this._y = p ? p._y + this.y : this.y;

		this._opacity = p ? p._opacity * this.opacity : this.opacity;

		this._zIndex = p ? p._zIndex + this._rIndex + this.zIndex : 
						   this._rIndex + this.zIndex;

		this._visible = p ? p._visible && this.visible : this.visible;
		this._overflow = p ? p._overflow && this.overflow : this.overflow;
		this._fixed = p ? p._fixed && this.fixed : this.fixed;

		// clipping area
		var scrollY = this._fixed===false ? (p ? p.scroll._top : 0) : 0,
			scrollX = 0;

		this._miny = this._y - scrollY;
		this._maxy = this._y+this.h - scrollY;

		this._minx = this._x - scrollX;
		this._maxx = this._x+this.w - scrollX;

		if (p && p._overflow === false) {
			if (p && this._miny<p._miny) this._miny = p._miny;
			if (p && this._maxy>p._maxy) this._maxy = p._maxy;

			if (p && this._minx<p._minx) this._minx = p._minx;
			if (p && this._maxx>p._maxx) this._maxx = p._maxx;

			this.offscreen = false;

			if (this._maxy <= this._miny) {
				this._maxy = this._miny;
				this.offscreen = true;
			}

			if (this._maxx <= this._minx) {
				this._maxx = this._minx;
				this.offscreen = true;
			}

			this.parent.clipping = {
				x : p._minx,
				y : p._miny,
				w : p._maxx - p._minx,
				h : p._maxy - p._miny
			};

		}

		// rotation inheritance
		this._protate = p ? p._rotate : this.rotate;
		this._rotate = this._protate + this.rotate;
		
		// scale inheritance
		this._pscale = p ? p._scale : this.scale;
		this._scale =  this._pscale * this.scale;

		// translation inheritance
		this.t._x = p ? p.t._x + this.t.x : this.t.x;
		this.t._y = p ? p.t._y + this.t.y : this.t.y;

		// gravity center before transformation
		this._g.x = (this._x + this.g.x + this.w/2);
		this._g.y = (this._y + this.g.y + this.h/2);

		// gravity center after scale + translation
		this._gs = {
			x : (this._g.x - this.t._x)*this._pscale,
			y : (this._g.y - this.t._y)*this._pscale
		};
	},

	__projection : function(x, y){
		var k = {
			x : this.parent ? this.parent.t._x : this.t.x,
			y : this.parent ? this.parent.t._y : this.t.y
		};

		return {
			x : (this._g.x - k.x)*this._pscale - (this._g.x - x)*this._scale,
			y : (this._g.y - k.y)*this._pscale - (this._g.y - y)*this._scale
		};
	},

	beforeDraw : function(){

		this._oldy = this._y;
		this._y = this._y - (this._fixed===false ? this.scroll._top : 0);

		if (this.clip){
			canvas.save();
			canvas.clipbox(
				this.clip.x, 
				this.clip.y, 
				this.clip.w, 
				this.clip.h, 
				this.radius
			);
			canvas.clip();
		}

		if (this.backgroundBlur){
			canvas.blur(
				this.blurbox.x, 
				this.blurbox.y, 
				this.blurbox.w, 
				this.blurbox.h, 
				this.backgroundBlur
			);
		}

		var DX = this._g.x - this.t._x,
			DY = this._g.y - this.t._y;

		// new coordinates after scale + translation
		var p = this.__projection(this._x, this._y);
		this.__x = p.x;
		this.__y = p.y;
		this.__w = this.w * this._scale;
		this.__h = this.h * this._scale;

		// new coordinates after scale + translation
		var m = this.__projection(this._maxx, this._maxy);
		this.__maxx = m.y;
		this.__maxy = m.y;

		this.t._x += (DX - DX/this.scale);
		this.t._y += (DY - DY/this.scale);

		/*		
		if (this._rotate!=0){
			canvas.save();
			this.rx = this._gs.x;
			this.ry = this._gs.y;

			canvas.translate(this.rx, this.ry);
			canvas.rotate(this.rotate*Math.PI/180);
			canvas.translate(-this.rx, -this.ry);

			this.rt = {
				x : (parent ? parent._gs.x : this._gs.x),
				y : (parent ? parent._gs.y : this._gs.y)
			};
			this.angle = this._protate*Math.PI/180;

			canvas.translate(this.rt.x, this.rt.y);
			canvas.rotate(this.angle);
			canvas.translate(-this.rt.x, -this.rt.y);

		}
		*/

		/* scale */

		if (this._scale != 1){
			canvas.save();
			canvas.scale(this._scale, this._scale);
			canvas.translate( -this.t._x, -this.t._y);
		}


		if (this.parent && this.parent._overflow === false){
			canvas.save();

			/*			
			canvas.roundbox(
				this.parent.clipping.x, this.parent.clipping.y,
				this.parent.clipping.w, this.parent.clipping.h,
				this.parent.radius, "rgba(255, 0, 0, 0.05)", false
			);
			*/

			canvas.clipbox(
				this.parent.clipping.x, this.parent.clipping.y,
				this.parent.clipping.w, this.parent.clipping.h,
				this.parent.radius
			);

			canvas.clip();
		}


		canvas.oldGlobalAlpha = canvas.globalAlpha;
		canvas.globalAlpha = this._opacity;

		if (this.hasFocus && this.flags._canReceiveFocus && this.flags._outlineOnFocus) {
			this.drawFocus();
		}

	},

	draw : function(){},

	drawFocus : function(){
		var p = {
				x : this._x,
				y : this._y,
				w : this.w,
				h : this.h
			},
			r = this.r+1;

		if (this.type=="UIText" || this.type=="UIWindow" || this.type=="UIDiagram") {
			/*
			canvas.setShadow(0, 0, 2, "rgba(255, 255, 255, 1)");
			canvas.roundbox(p.x, p.y, p.w, p.h, r, "rgba(0, 0, 0, 0.0)", "#ffffff");
			canvas.setShadow(0, 0, 4, "rgba(80, 190, 230, 1)");
			canvas.roundbox(p.x, p.y, p.w, p.h, r, "rgba(0, 0, 0, 0.0)", "#4D90FE");
			canvas.setShadow(0, 0, 5, "rgba(80, 190, 230, 1)");
			canvas.roundbox(p.x, p.y, p.w, p.h, r, "rgba(0, 0, 0, 0.0)", "#4D90FE");
			canvas.setShadow(0, 0, 0);
			*/
		}

		else if (this.type == "UILine") {
			canvas.save();
			canvas.strokeStyle = "rgba(0, 0, 0, 0.10)";
			canvas.lineWidth = this.lineWidth+20;
			canvas.spline(this.path);
			canvas.restore();
		}

	},

	afterDraw : function(){
		this._y = this._oldy;

		if (this.callback) this.callback.call(this);
		canvas.globalAlpha = canvas.oldGlobalAlpha;

		/*
		if (this._rotate!=0){

			canvas.translate(this.rt.x, this.rt.y);
			canvas.rotate(-this.angle);
			canvas.translate(-this.rt.x, -this.rt.y);

			canvas.translate(this.rx, this.ry);
			canvas.rotate(-this.rotate*Math.PI/180);
			canvas.translate(-this.rx, -this.ry);

			canvas.restore();
		}
		*/


		if (this.parent && this.parent._overflow === false){
			canvas.restore();
		}


		if (this._scale !=1 ){
			canvas.translate(this.t._x, this.t._y);
			canvas.scale(1/this._scale, 1/this._scale);
			canvas.restore();
		}

		/*
        canvas.beginPath();
        canvas.arc(this._g.x, this._g.y, 2, 0, 6.2831852, false);
        canvas.setColor("#ff0000");
        canvas.fill();
        canvas.lineWidth = 1;
        canvas.strokeStyle = "rgba(140, 140, 140, 0.7)";
        canvas.stroke();
        */

		//gravity center after scale + translation
		/*
		this._gs = {
			x : (this._g.x - this.t._x)*this._pscale,
			y : (this._g.y - this.t._y)*this._pscale
		};

        canvas.beginPath();
        canvas.arc(this._gs.x, this._gs.y, 2, 0, 6.2831852, false);
        canvas.setColor("#00ff00");
        canvas.fill();
        canvas.lineWidth = 1;
        canvas.strokeStyle = "rgba(140, 140, 140, 0.7)";
        canvas.stroke();
        */

		if (this.clip){
			canvas.restore();
		}

	},

	/* -------------------------------------------------------------- */

	isPointInside : function(mx, my){
		var x1 = this.__x,
			y1 = this.__y,
			x2 = x1 + this.__w,
			y2 = this.__maxy; //y1 + this.__h;

		return	(mx>=x1 && mx<x2 && my>=y1 && my<y2) ? true : false;
	},

	isVisible : function(){
		var mx = window.width,
			my = window.height,

			x1 = this.__x,
			x2 = this.__x+this.__w,
			y1 = this.__y,
			y2 = this.__y+this.__y;

		return this._visible;
	},

	/* -------------------------------------------------------------- */

	get left() {
  		return this.x;
	},

	set left(value) {
		var dx = value - this.x;
		if (dx==0) { return false; }
		this._x += dx;
		this.x += dx;
		Native.layout.refresh();
	},

	get top() {
  		return this.y;
	},

	set top(value) {
		var dy = value - this.y;
		if (dy==0) { return false; }
		this._y += dy;
		this.y += dy;
		Native.layout.refresh();
	},

	get transformOrigin() {
  		return this._g;
	},

	set transformOrigin(g) {
		var ox = this.g.x,
			oy = this.g.y;

		this.g = {
			x : OptionalNumber(g.x, ox) - this._x - this.w/2,
			y : OptionalNumber(g.y, oy) - this._y - this.h/2
		}
		Native.layout.refresh();
	}

};

DOMElement.implement = function(props){
	for (var key in props){
		if (props.hasOwnProperty(key)){
			DOMElement.prototype[key] = props[key];
		}
	}
};

/* Map ES5 Property Descriptor to our DOMElement object */
DOMElement.defineProperty = function(property, defaultValue, readOnly){
	Object.defineProperty(this.prototype, property, {
		value : defaultValue,
		enumerable : true,
		writable : readOnly === undefined ? true : !readOnly,
		configurable : false
	});
};

DOMElement.getPropertyDescriptor = function(property){
	return Object.getOwnPropertyDescriptor(this.prototype, property);
};

DOMElement.proxy = function(obj, handler){
	return Proxy.create(handler, obj, Object.getPrototypeOf(obj));
};

Native.elements = {
	export : function(type, implement){
		if (type=="export" || type=="export" || type=="export"){
			return false;
		}
		this[type] = implement;
		this.build(Object.scope, type);
	},

	build : function(scope, name){
		var f = function(parent, options){
			return parent.add(name, options);
		};
		scope[String(name)] = f;
	},

	init : function(element){
		var self = this,
			plugin = this[element.type];

		if (plugin){

			if (plugin.init) plugin.init.call(element);
			if (plugin.draw) element.draw = plugin.draw;
			if (plugin.isPointInside) {
				element.isPointInside = plugin.isPointInside;
			}
			if (plugin.__construct) element.__construct = plugin.__construct;

			if (element.flags._canReceiveFocus) {
				element.addEventListener("mousedown", function(e){
					this.focus();
					e.stopPropagation();
				}, false);
			}

		} else {
			element.beforeDraw = function(){};
			element.draw = function(){};
			element.afterDraw = function(){};
		}
	}
};

var Application = function(options){
	var app = new DOMElement("UIView", options, null);
	app._root = true;
	app.flags._canReceiveFocus = true;
	app.flags._outlineOnFocus = false;

	Native.elements.init(app);
	Native.layout.rootElement = app;
	Native.layout.register(app);
	Native.layout.refresh();

	canvas.globalAlpha = 1;
	canvas.__mustBeDrawn = true;

	var bgCanvas = new Image();
	bgCanvas.src = "demos/assets/spheres.jpeg";

	if (options && options.animation===false){
		/* dummy */
	} else {
		canvas.animate = true;
	    canvas.requestAnimationFrame(function(){
			FPS.start();
	 		if (canvas.animate) {

				canvas.drawImage(bgCanvas, 0, 0);
				if (Native.layout.hook) Native.layout.hook();
				Native.layout.draw();

				if (window && window.requestAnimationFrame){
					window.requestAnimationFrame();
				}

				//Native.layout.grid();
			} 
	 		FPS.show();
	    });
	}


	return app;
};


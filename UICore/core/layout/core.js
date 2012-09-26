/* -------------------------- */
/* Native (@) 2012 Stight.com */
/* -------------------------- */

var UIView = function(type, options, parent){
	var view = this;
	this.options = options || {};
	this.parent = parent ? parent : null; // parent element
	this.nodes = {}; // children elements

	if (!UIElement[type]) {
		throw("Unknown element " + type);
	}

	var _get = function(property, defaultValue, min, max){
		var p = view.options[property] ? view.options[property] : defaultValue;
		if (typeof p == "number") {
			p = (typeof min != "undefined") ? Math.max(min, p) : p;
			p = (typeof max != "undefined") ? Math.min(max, p) : p;
		}
		return p;
	};

	this._uid = "_obj_" + NativeRenderer.objID++;
	this.id = _get("id", this._uid);

	this.type = type ? type : "UIView";
	this.name = _get("name", "");
	this.text = _get("text", "");
	this.label = _get("label", "Default");

	this._eventQueues = [];

	// -- coordinate properties
	this.x = _get("x", 0);
	this.y = _get("y", 0);
	this.w = this.options.w ? this.options.w : this.parent ? this.parent.w : window.width;
	this.h = this.options.h ? this.options.h : this.parent ? this.parent.h : window.height;

	this.rotate = 0;
	this.scale = _get("scale", 1, 0, 1000); //this.options.scale ? parseFloat(this.options.scale) : 1;
	this.zIndex = _get("zIndex", 0); //this.options.zIndex ? Math.round(this.options.zIndex) : 0;
	this.opacity = _get("zIndex", 1, 0, 1); // this.options.opacity ? parseFloat(this.options.opacity) : 1;

	if (this.parent) {
		this._rIndex = NativeRenderer.getHigherZindex() + 1;
	} else {
		this._rIndex = 0;
	}

	// -- dynamic properties (properties prefixed by _ inherits from parent at draw time)
	this._rotate = this.rotate;
	this._scale = this.scale;
	this._x = this.x;
	this._y = this.y;
	this._opacity = this.opacity;
	this._zIndex = this._rIndex + this.zIndex;

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

	// -- events flags
	this.flags = {
		_mouseoverCalled : false,
		_mouseoutCalled : false,
		_dragendCallend : false,
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


	// -- misc flag
	this.hover = false;
	this.hasFocus = false;
	this.visible = (this.options.visible==undefined) ? true : (this.options.selected===false) ? false : true;
	this.selected = (this.options.selected==undefined) ? false : (this.options.selected===false) ? false : true;
	this.draggable = (this.options.draggable==undefined) ? false : (this.options.draggable===false) ? false : true;

	// -- style properties
	this.background = (this.options.background && this.options.background!='') ? this.options.background : '';
	this.color = _get("color", "");
	this.radius = _get("radius", 0, 0);
	this.shadowBlur = _get("shadowBlur", 0, 0, 128);
	this.lineWidth = _get("lineWidth", 1, 0);
	this.lineHeight = _get("lineHeight", 18, 1, this.fontSize);
	this.fontSize = _get("fontSize", 12, 0, 74);
	this.fontType = _get("fontType", "arial");

	var align = this.options.textAlign && typeof(this.options.textAlign=="string") ? this.options.textAlign.toLowerCase() : "left";
	this.textAlign = align && (align=="left" || align=="right" || align=="justify" || align=="center") ? align : 'left';


	// -- launch view constructor and init dynamic properties
	this.__construct();
	this.__refreshDynamicProperties();
};

UIView.prototype = {
	__construct : function(){

	},

	add : function(type, options){
		var view = new UIView(type, options, this);

		UIElement.init(view);
		this.addChild(view);
		return view;

		/*
		view.__defineGetter__("nbnodes", function() {
			return count(view.nodes);
		});
		*/
	},

	clone : function(){
		var view = new UIView(this.type, this.options, this.parent);
		for (var i in this){
			view[i] = this[i];
		}

		view._uid = this._uid + "_clone";
		view.id = view._uid;

		view._rIndex = NativeRenderer.getHigherZindex() + 1;
		view.opacity = 0.8;
		view.nodes = {}; // kill children nodes

		UIElement.init(view);
		NativeRenderer.register(view);

		return view;
	},

	addChild : function(view){
		this.nodes[view._uid] = view;
		NativeRenderer.refresh();
	},

	remove : function(){
		NativeRenderer.remove(this);
	},

	focus : function(){
		if (this.hasFocus === true) {
			return false;
		}

		if (this.flags._canReceiveFocus) {
			this.hasFocus = true;
	
			this.fireEvent("focus", {});

			if (NativeRenderer.lastFocusedElement) {
				NativeRenderer.lastFocusedElement.fireEvent("blur", {});
			}

			NativeRenderer.lastFocusedElement = this;

			NativeRenderer.focusObj = this._nid;
		}
	},

	show : function(){
		if (!this.visible) {
			this.visible = true;
			NativeRenderer.refresh();
		}
	},

	hide : function(){
		if (this.visible) {
			this.visible = false;
			NativeRenderer.refresh();
		}
	},

	bringToTop : function(){
		this.zIndex = NativeRenderer.getHigherZindex() + 1;
	},


	__refreshDynamicProperties : function(){
		// -- dynamic properties
		// -- properties prefixed by _ inherits from parent at draw time
		var parent = this.parent;

		this._x = parent ? parent._x + this.x : this.x;
		this._y = parent ? parent._y + this.y : this.y;
		this._opacity = parent ? parent._opacity * this.opacity : this.opacity;
		this._zIndex = parent ? parent._zIndex + this._rIndex + this.zIndex : this._rIndex + this.zIndex;

		// rotation inheritance
		this._protate = parent ? parent._rotate : this.rotate;
		this._rotate = this._protate + this.rotate;
		
		// scale inheritance
		this._pscale = parent ? parent._scale : this.scale;
		this._scale =  this._pscale * this.scale;

		// translation inheritance
		this.t._x = parent ? parent.t._x + this.t.x : this.t.x;
		this.t._y = parent ? parent.t._y + this.t.y : this.t.y;

		// gravity center before transformation
		this._g.x = (this._x + this.g.x + this.w/2);
		this._g.y = (this._y + this.g.y + this.h/2);

		// gravity center after scale + translation
		this._gs = {
			x : (this._g.x - this.t._x)*this._pscale,
			y : (this._g.y - this.t._y)*this._pscale
		};
	},

	beforeDraw : function(){
		this.__refreshDynamicProperties();

		if (this.clip){
			canvas.save();
			canvas.roundbox(this.clip.x, this.clip.y, this.clip.w, this.clip.h, 0, false, false); // main view
			canvas.clip();
		}

		var DX = this._g.x - this.t._x,
			DY = this._g.y - this.t._y;

		// new coordinates after scale + translation
		var p = this.__projection(this._x, this._y); // p is the new coords after scale + translation
		this.__x = p.x;
		this.__y = p.y;
		this.__w = this.w * this._scale;
		this.__h = this.h * this._scale;

		this.t._x += (DX - DX/this.scale); // dt.x
		this.t._y += (DY - DY/this.scale); // dt.y

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
		if (this._scale!=1){
			canvas.save();
			canvas.scale(this._scale, this._scale);
			canvas.translate( -this.t._x, -this.t._y);

		}
		/* scale */

		canvas.oldGlobalAlpha = canvas.globalAlpha;
		canvas.globalAlpha = this._opacity;

		if (this.hasFocus && this.flags._canReceiveFocus && this.flags._outlineOnFocus) {
			var params = {
					x : this._x,
					y : this._y,
					w : this.w,
					h : this.h
				},
				radius = this.radius+1;

			if (this.type=="UIText" || this.type=="UIWindow") {
				canvas.setShadow(0, 0, 2, "rgba(255, 255, 255, 1)");
				canvas.roundbox(params.x, params.y, params.w, params.h, radius, "rgba(0,0,0,1)", "#ffffff");
				canvas.setShadow(0, 0, 4, "rgba(80, 190, 230, 1)");
				canvas.roundbox(params.x, params.y, params.w, params.h, radius, "rgba(0,0,0,0.8)", "#4D90FE");
				canvas.setShadow(0, 0, 5, "rgba(80, 190, 230, 1)");
				canvas.roundbox(params.x, params.y, params.w, params.h, radius, "rgba(0,0,0,0.6)", "#4D90FE");
				canvas.setShadow(0, 0, 0);
			}
		}


	},

	draw : function(){},

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

	afterDraw : function(){

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

		if (this._scale!=1){
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
			y2 = y1 + this.__h;

		return	(mx>=x1 && mx<=x2 && my>=y1 && my<=y2) ? true : false;
	},

	/* -------------------------------------------------------------- */

	get left() {
  		return this.x;
	},

	set left(value) {
		var dx = value - this.x;
		if (dx==0) {return false;}
		this._x += dx;
		this.x += dx;
		NativeRenderer.refresh();
	},

	get top() {
  		return this.y;
	},

	set top(value) {
		var dy = value - this.y;
		if (dy==0) {return false;}
		this._y += dy;
		this.y += dy;
		NativeRenderer.refresh();
	}

};

UIView.implement = function(props){
	for (var key in props){
		if (props.hasOwnProperty(key)){
			UIView.prototype[key] = props[key];
		}
	}
};

var UIElement = {
	extend : function(UIElement, implement){
		this[UIElement] = implement;
	},

	init : function(UIView){
		var self = this,
			UIElement = UIView.type;

		if (this[UIElement]){

			if (this[UIElement].init) this[UIElement].init.call(UIView);
			if (this[UIElement].draw) UIView.draw = this[UIElement].draw;
			if (this[UIElement].isPointInside) UIView.isPointInside = this[UIElement].isPointInside;
			if (this[UIElement].__construct) UIView.__construct = this[UIElement].__construct;

			if (UIView.flags._canReceiveFocus) {
				UIView.addEventListener("mousedown", function(e){
					this.focus();
					e.stopPropagation();
				}, false);
			}

		} else {
			UIView.beforeDraw = function(){};
			UIView.draw = function(){};
			UIView.afterDraw = function(){};
		}
	}
};

var Application = function(options){
	var view = new UIView("UIView", options, null);
	view._root = true;
	view.flags._canReceiveFocus = true;
	view.flags._outlineOnFocus = false;
	UIElement.init(view);

	NativeRenderer.register(view);
	NativeRenderer.refresh();

	NativeRenderer.rootElement = view;

	/*
	var bgCanvas = new Image(),
		background = new Canvas(1024, 768),
		backgroundData;

	bgCanvas.src = "demos/assets/flavor.jpeg";
	background.drawImage(bgCanvas, 0, 0);
	backgroundData = background.getImageData(0, 0, 1024, 768);
	*/

	canvas.globalAlpha = 1;
	canvas.__mustBeDrawn = true;

	if (options && options.animation===false){
		/* dummy */
	} else {
		canvas.animate = true;
	    canvas.requestAnimationFrame(function(){
			FPS.start();
	 		if (canvas.animate) {

	 			//canvas.putImageData(backgroundData, 0, 0);
				NativeRenderer.draw();
				//NativeRenderer.grid();
			} 
	 		FPS.show();
	    });
	}


	return view;
};
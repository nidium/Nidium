/* -------------------------- */
/* Native (@) 2012 Stight.com */
/* -------------------------- */

var UIView = function(type, options){
	this.options = options || {};

	if (!UIElement[type]) {
		throw("Unknown element " + type);
	}

	this.id = '_obj_' + layout.objID;
	this.type = type || 'UIView';
	this.text = '';
	this.name = this.options.name || "";

	this.x = this.options.x ? this.options.x : 0;
	this.y = this.options.y ? this.options.y : 0;
	this.w = this.options.w ? this.options.w : this.parent ? this.parent.w : window.width;
	this.h = this.options.h ? this.options.h : this.parent ? this.parent.h : window.height;

	this.radius = this.options.radius ? this.options.radius : 0;

	this.scale = 1;
	this._scale = 1;

	this.shadowBlur = this.options.shadowBlur ? Math.round(this.options.shadowBlur) : 0;

	this.rotate = 0;
	this._rotate = 0;

	this._x = this.x;
	this._y = this.y;
	this._eventQueues = [];

	this.opacity = this.options.opacity ? this.options.opacity : 1;
	this._opacity = this.opacity;
	this._area = null;

	this.zIndex = 0;

	this.nodes = {};

	this.scroll = {
		top : 0,
		left : 0
	};

	this.content = {
		width : this.w,
		height : this.h
	};

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

	this.hover = false;
	this.selected = (this.options.selected==undefined) ? false : (this.options.selected===false) ? false : true;

	this.draggable = (this.options.draggable==undefined) ? false : (this.options.draggable===false) ? false : true;

	this.label = this.options.label ? this.options.label : 'Default';
	this.background = this.options.background ? this.options.background : '';
	this.color = this.options.color ? this.options.color : '';
	this.fontSize = this.options.fontSize ? this.options.fontSize : 12;
	this.lineWidth = this.options.lineWidth ? this.options.lineWidth : 1;
};
UIView.prototype = {
	id : '',
	parent : null,

	type : 'UIView',
	label : 'Default',
	text : '',
	name : '',

	visible : true,
	selected : false,
	hover : false,
	draggable : false,

	x : 0,
	y : 0,
	w : 0,
	h : 0,

	background : '',
	color : '',
	lineWidth : 1,
	fontSize : 12,
	radius : 0,
	zIndex : 0,

	shadowBlur : 0,

	scale : 1,
	_scale : 1,

	rotate : 0,
	_rotate : 0,

	_x : 0,
	_y : 0,
	
	scroll : {
		top : 0,
		left : 0
	},

	content : {
		width : 0,
		height : 0
	},

	g : {
		x : 0,
		y : 0
	},

	_g : {
		x : 0,
		y : 0
	},

	t : {
		_x : 0,
		_y : 0,
		x : 0,
		y : 0
	},

	_area : null,

	opacity : 1,
	_opacity : 1,

	_eventQueues : [],

	nodes : {},

	__construct : function(){

	},

	createElement : function(type, options){
		if (!options) {
			options = {};
		}

		var view = new UIView(type, options);

		view.parent = this;
		view.text = options.text || "";
		view.zIndex = view.parent.zIndex + 1;

		view._x = this._x + options.x;
		view._y = this._y + options.y;

		view.g = {
			x : 0,
			y : 0
		};

		view._g = {
			x : 0,
			y : 0
		};

		view._eventQueues = [];
		view._opacity = this._opacity * (options.opacity ? parseFloat(options.opacity) : 1);
		view.background = (options.background && options.background!='') ? options.background : '';

		UIElement.init(view);

		view.__defineGetter__("nbnodes", function() {
			return count(view.nodes);
		});

		view.flags = {
			_mouseoverCalled : false,
			_mouseoutCalled : false,
			_dragendCallend : false
		},

		view.__construct();

		this.addChild(view);

		return view;
	},

	clone : function(){
		var view = new UIView(this.type, this.options);
		for (var i in this){
			view[i] = this[i];
		}

		view.id = view.id + "_clone";
		view.zIndex = layout.getHigherZindex() + 1;
		view.nodes = {}; // kill children nodes

		view._opacity = 0.5;

		UIElement.init(view);
		layout.registerClone(view);

		return view;
	},

	addChild : function(view){
		this.nodes[view.id] = view;
		layout.objID++;
		layout.refresh();
	},

	beforeDraw : function(){
		this._x = (this.parent) ? this.parent._x + this.x : this.x;
		this._y = (this.parent) ? this.parent._y + this.y : this.y;
		this._opacity = (this.parent) ? this.parent._opacity * this.opacity : this.opacity;

		this._protate = (this.parent) ? this.parent._rotate : this.rotate;
		this._rotate = this._protate + this.rotate;
		
		this._pscale = (this.parent) ? this.parent._scale : this.scale;
		this._scale =  this._pscale * this.scale;

		this.t._x = (this.parent) ? this.parent.t._x + this.t.x : this.t.x;
		this.t._y = (this.parent) ? this.parent.t._y + this.t.y : this.t.y;


		if (this.clip){
			canvas.save();
			canvas.roundbox(this.clip.x, this.clip.y, this.clip.w, this.clip.h, 0, false, false); // main view
			canvas.clip();
		}

		// gravity center before transformation
		this._g.x = (this._x + this.g.x + this.w/2);
		this._g.y = (this._y + this.g.y + this.h/2);

		var DX = this._g.x - this.t._x,
			DY = this._g.y - this.t._y;


		// gravity center after scale + translation
		this._gs = {
			x : (this._g.x - this.t._x)*this._pscale,
			y : (this._g.y - this.t._y)*this._pscale
		};

		// new object details after scale + translation
		var p = this.__projection(this._x, this._y); // p is the new coords after scale + translation
		this.__x = p.x;
		this.__y = p.y;
		this.__w = this.w * this._scale;
		this.__h = this.h * this._scale;


		/*

		var dt = {
			x : (this._gs.x*this.scale - this._gs.x)/this._scale,
			y : (this._gs.y*this.scale - this._gs.y)/this._scale
		};
								|
								v
		var dt = {
			x : ( DX*S - DX*S*1/ts ) / S,
			y : ( DY*S - DY*S*1/ts ) / S
		};
								|
								v
		var dt = {
			x : S( DX - DX/ts ) / S   =  DX - DX/ts,
			y : S( DY - DY/ts ) / S   =  DY - DY/ts
		};

		*/

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
				x : (this.parent ? this.parent._gs.x : this._gs.x),
				y : (this.parent ? this.parent._gs.y : this._gs.y)
			};
			this.angle = this._protate*Math.PI/180;

			canvas.translate(this.rt.x, this.rt.y);
			canvas.rotate(this.angle);
			canvas.translate(-this.rt.x, -this.rt.y);

		}
		*/

		/* scale */
		//if (this._scale!=1){
			canvas.save();
			canvas.scale(this._scale, this._scale);
			canvas.translate( -this.t._x, -this.t._y);

		//}
		/* scale */

		canvas.oldGlobalAlpha = canvas.globalAlpha;
		canvas.globalAlpha = this._opacity;

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

		//if (this._scale!=1){
			canvas.translate(this.t._x, this.t._y);
			canvas.scale(1/this._scale, 1/this._scale);
			canvas.restore();
		//}

		/*
        canvas.beginPath();
        canvas.arc(this._g.x, this._g.y, 2, 0, 6.2831852, false);
        canvas.fillStyle = "#ff0000";
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
        canvas.fillStyle = "#00ff00";
        canvas.fill();
        canvas.lineWidth = 1;
        canvas.strokeStyle = "rgba(140, 140, 140, 0.7)";
        canvas.stroke();
        */

		if (this.clip){
			canvas.restore();
		}

	},

	remove : function(){
		layout.remove(this);
	},

	show : function(){
		if (!this.visible) {
			this.visible = true;
			layout.refresh();
		}
	},

	hide : function(){
		if (this.visible) {
			this.visible = false;
			layout.refresh();
		}
	},

	onmouseover : null,
	onmouseout : null,
	onmousedown : null,

	flags : {
		_mouseoverCalled : false,
		_mouseoutCalled : false,
		_dragendCallend : false
	},

	fireEvent : function(eventName, e){
			canvas.__mustBeDrawn = true;
		if (typeof this["on"+eventName] == 'function'){
			this["on"+eventName](e);
		}
	},

	throwMouseOver : function(e){
		if (!this.flags._mouseoverCalled) {
			this.flags._mouseoverCalled = true;
			this.flags._mouseoutCalled = false;
			
			if (canvas.global.__dragging) {
				if (this.ondragenter && typeof(this.ondragenter)=="function"){
					this.ondragenter(e);
					canvas.__mustBeDrawn = true;
					return false;
				}
			} else {
				if (this.onmouseover && typeof(this.onmouseover)=="function"){
					this.onmouseover(e);
					canvas.__mustBeDrawn = true;
					return false;
				}
			}
		}
	},

	throwMouseOut : function(e){
		if (this.flags._mouseoverCalled && !this.flags._mouseoutCalled) {
			this.flags._mouseoverCalled = false;
			this.flags._mouseoutCalled = true;
			if (canvas.global.__dragging) {

				/* to check */
				if (this.onmouseout && typeof(this.onmouseout)=="function"){
					this.onmouseout(e);
					canvas.__mustBeDrawn = true;
				}
				if (this.onmouseleave && typeof(this.onmouseleave)=="function"){
					this.onmouseleave(e);
					canvas.__mustBeDrawn = true;
				}
				/* -------- */

				if (this.ondragleave && typeof(this.ondragleave)=="function"){
					this.ondragleave(e);
					canvas.__mustBeDrawn = true;
					return false;
				}
			} else {
				if (this.onmouseout && typeof(this.onmouseout)=="function"){
					this.onmouseout(e);
					canvas.__mustBeDrawn = true;
				}
				if (this.onmouseleave && typeof(this.onmouseleave)=="function"){
					this.onmouseleave(e);
					canvas.__mustBeDrawn = true;
				}
				return true;
			}
		}
	},

	addEventListener : function(eventName, callback, propagation){
		var self = this;

		propagation = (typeof propagation == "undefined") ? true : (propagation===true) ? true : false;
		

		if (!self._eventQueues[eventName]) {
			self._eventQueues[eventName] = [];
		}

		self._eventQueues[eventName].push({
			name : eventName,
			throwEvent : callback,
			propagation : true
		});

		self["on"+eventName] = function(e){
			for(var i in self._eventQueues[eventName]){
				self._eventQueues[eventName][i].throwEvent.call(self, e);
				if (self._eventQueues[eventName][i].propagation===false){
					break;
				}
			}
		};

	},

	scrollY : function(deltaY){
		var self = this,
			startY = this.scroll.top,
			endY = this.scroll.top + deltaY,
			maxY = this.content.height-this.h,
			slice = 10,
			duration = 80;

		if (!this.scroll.initied){
			self.scroll.initied = true;
			self.scroll.time = 0;
			self.scroll.duration = duration;
			self.scroll.startY = startY;
			self.scroll.endY = endY;
			self.scroll.deltaY = deltaY;
			self.scroll.accy = 1.0;
		}

		if (this.scroll.scrolling) {
			Timers.remove(self.scroll.timer);
			this.scroll.scrolling = false;
			this.scroll.initied = false;
		}

		if (!this.scroll.scrolling){


			self.scroll.scrolling = true;

			self.scroll.timer = setTimeout(function(){
				let stop = false;
		
				self.scroll.top = FXAnimation.easeOutCubic(0, self.scroll.time, startY, deltaY, self.scroll.duration);
				self.scroll.time += slice;

				delete(self.__cache);
				canvas.__mustBeDrawn = true;


				if (deltaY>=0) {
					if (self.scroll.top > maxY) {
						self.scroll.top = maxY;
						stop = true;
					}

					if (self.scroll.top > endY) {
						self.scroll.top = endY;
						stop = true;
					}
				} else {
					if (self.scroll.top < endY) {
						self.scroll.top = endY;
						stop = true;
					}

					if (self.scroll.top < 0) {
						self.scroll.top = 0;
						stop = true;
					}
				}

				if (self.scroll.time>duration){
					stop = true;
				}

				if (stop && this.remove){
					self.scroll.scrolling = false;
					self.scroll.initied = false;
					this.remove();
				}

			}, slice, true, true);

		}

	},

	isPointInside : function(mx, my){
		var x1 = this.__x,
			y1 = this.__y,
			x2 = x1 + this.__w,
			y2 = y1 + this.__h;

		return	(mx>=x1 && mx<=x2 && my>=y1 && my<=y2) ? true : false;
	},

	get left() {
  		return this.x;
	},

	set left(value) {
		var dx = value - this.x;
		if (dx==0) {return false;}
		this._x += dx;
		this.x += dx;
		layout.refresh();
	},

	get top() {
  		return this.y;
	},

	set top(value) {
		var dy = value - this.y;
		if (dy==0) {return false;}
		this._y += dy;
		this.y += dy;
		layout.refresh();
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
		let self = this,
			UIElement = UIView.type;

		if (this[UIElement]){

			this[UIElement].init.call(UIView);
			
			if (this[UIElement].draw) UIView.draw = this[UIElement].draw;
			if (this[UIElement].isPointInside) UIView.isPointInside = this[UIElement].isPointInside;
			if (this[UIElement].__construct) UIView.__construct = this[UIElement].__construct;

		} else {
			UIView.beforeDraw = function(){};
			UIView.draw = function(){};
			UIView.afterDraw = function(){};
		}
	}
};

var Application = function(options){
	var view = new UIView("UIView", options);

	UIElement.init(view);

	layout.register(view);
	layout.refresh();

	
	/*
	var z = new Image();
	z.src = "assets/bg01.png";
	*/

	canvas.globalAlpha = 1;

	if (options && options.animation===false){
		/* dummy */
	} else {
		var __DATE__,
			__FPS__ = 0,
			__FPS_OLD__ = 0;

		canvas.showFPS = function(){
			__FPS__++;
			let r = 2 + (+ new Date()) - __DATE__,
				fps = Math.round(1000/r);

			//echo(r);
			if (__FPS__%30==0){
				__FPS_OLD__ = fps;
			} 				

			canvas.fillStyle = "black";
			canvas.fillRect(0, canvas.height-40, 60, 30);
			canvas.fillStyle = "yellow";
			canvas.fillText(__FPS_OLD__ + " FPS", 5, canvas.height-20);

			return r;
		};

		canvas.animate = true;
	    canvas.requestAnimationFrame(function(){
			Timers.manage();
			
			__DATE__ = (+ new Date());
	 		if (canvas.animate) {
				layout.draw();
				//layout.grid();
				//canvas.drawImage(z, 0, 0, 1124, 868);
			}
			//canvas.blur(0, 0, 1024, 768, 2);
	 		canvas.showFPS();

			canvas.fillStyle = "black";
			canvas.fillRect(0, 280, 50, 30);
	    });
	}


	return view;
};



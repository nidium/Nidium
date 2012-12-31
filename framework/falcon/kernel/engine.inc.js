/* -------------------------- */
/* Native (@) 2013 Stight.com */
/* -------------------------- */

"use strict";

/* -------------------------------------------------------------------------- */

Native.scope = this;
Native._cachedTextWidth = {};
Native.blankOrphanedCanvas = new Canvas(1, 1);
Native.canvas.context = Native.canvas.getContext("2D");
Native.canvas.implement = function(props){
	Object.merge(CanvasRenderingContext2D.prototype, props);
};

/* -------------------------------------------------------------------------- */

Native.elements = {
	export : function(type, implement){
		if (type=="export" || type=="build" || type=="init") return false;
		this[type] = implement;
		this.build(Native.scope, type);
	},

	build : function(scope, name){
		scope[String(name)] = function(parent, options){
			return parent.add(name, options);
		};
	},

	createHardwareLayer : function(element){
		var w = element.getLayerPixelWidth(),
			h = element.getLayerPixelHeight();

		element.layer = new Canvas(w, h);
		element.layer.context = element.layer.getContext("2D");
	},

	init : function(element){
		var plugin = this[element.type];

		element.__lock();

		if (element.parent){
			this.createHardwareLayer(element);
		} else {
			element._layerPadding = 0;
			element._root = element;
			this.createHardwareLayer(element);

			Native.canvas.add(element.layer);
			Native.layout.register(element);
		}

		element.layer.host = element;

		if (plugin){
			if (plugin.draw) element.draw = plugin.draw;
			if (plugin.refresh) element.update = plugin.refresh;

			if (plugin.public) {
				DOMElement.defineDescriptors(element, plugin.public);
			}

			if (plugin.init) plugin.init.call(element);

		} else {
			element.draw = function(context){};
		}

		if (element.canReceiveFocus) {
			element.addEventListener("mousedown", function(e){
				this.focus();
				e.stopPropagation();
			}, false);
		}

		DOMElement.defineReadOnlyProperties(element, {
			loaded : true
		});

		if (typeof(element.onReady) == "function"){
			element.onReady.call(element);
		}

		element.__unlock();

	}
};

/* -------------------------------------------------------------------------- */

Native.getTextWidth = function(text, fontSize, fontType){
	var c = Native._cachedTextWidth,
		key = text + fontSize + fontType,
		context = Native.blankOrphanedCanvas.getContext("2D");

	if (!c[key]) {
		context.fontSize = fontSize;
		context.fontType = fontType;
		c[key] = context.measureText(text);
	}

	return c[key];
};

/* -------------------------------------------------------------------------- */

Native.loadImage = function(url, callback){
	var cb = OptionalCallback(callback, function(){}),
		img = new Image();

	img.onload = function(){
		cb(img);
	};
	img.src = url;
};

/* -------------------------------------------------------------------------- */

Native.timer = function(fn, ms, loop, execFirst){
	var t = {
		loop : loop,
		tid : loop 
			? setInterval(function(){fn.call(t);}, ms)
			: setTimeout(function(){fn.call(t);}, ms),

		remove : function(){
			if (this.loop) {
				clearInterval(this.tid);
			} else {
				clearTimeout(this.tid);
			}
			delete(this.tid);
		}
	};

	if (execFirst) {
		fn.call(t);
	}
	
	return t;
};

/* -------------------------------------------------------------------------- */

Native.FPS = {
	init : function(){},
	start : function(){},
	show : function(){}
};

/* -------------------------------------------------------------------------- */

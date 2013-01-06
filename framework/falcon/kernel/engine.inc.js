/* ------------------------+------------- */
/* Native Framework 2.0    | Falcon Build */
/* ------------------------+------------- */
/* (c) 2013 Stight.com - Vincent Fontaine */
/* -------------------------------------- */

"use strict";

/* -------------------------------------------------------------------------- */

Native.scope = this;
Native._cachedTextWidth = {};
Native.blankOrphanedCanvas = new Canvas(1, 1);
Native.canvas.context = Native.canvas.getContext("2D");

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

Object.merge(File.prototype, {
	offset : 0,

	load : function(callback){
		if (typeof callback != "function" && typeof this.onload != "function"){
			throw "File read : callback parameter or onload method expected";
		}

		callback = callback ? callback : this.onload;

		if (this.mutex) {
			throw new Error("File locked until callback");
			return false;
		}

		this.mutex = true;

		this.open(function(){
			this.read(5000000, function(buffer){
				this.mutex = false;
				this.offset = 0;
				this.size = buffer.byteLength;
				this.buffer = buffer;
				callback.call(this, this.buffer, this.size);
			});
		});
	},

	get : function(...n){
		var view = this.buffer,
			offset = 0,
			size = 0,
			maxSize = 0;

		if (!this.buffer) {
			throw new Error("File read : can't access uninitialized file");
		}

		switch (n.length){
			case 1 :
				/* f.read(size) */
				offset = this.offset;
				size = OptionalNumber(n[0], null);
				break;

			case 2 :
				/* f.read(offset, size) */
				offset = OptionalNumber(n[0], 0);
				size = OptionalNumber(n[1], null);
			 	break;

			 default :
				throw new Error("File read : missing parameters");
			 	break;
		}

		if (size == null){
			throw new Error("File read : expected size");
		}

		offset = offset.bound(0, this.size-1);
		maxSize = this.size-offset;

		size = size == null ? maxSize : size;
		size = size.bound(1, maxSize);
		
		if (offset == 0 && size == this.size){
			view = this.buffer;
		} else {
			view = new Uint8Array(this.buffer, offset, size);
		}

		this.offset = offset + size;
		return view;
	}
});

File.getText = function(url, callback){
	var f = new File(url);
	f.open(function(){
		f.read(5000000, function(buffer){
			this.size = buffer.byteLength;
			this.buffer = buffer;
			if (typeof callback == "function") callback.call(this, this.buffer.toString());
		});
	});
};

File.read = function(url, callback){
	var f = new File(url);
	f.open(function(){
		f.read(5000000, function(buffer){
			this.size = buffer.byteLength;
			this.buffer = buffer;
			if (typeof callback == "function") callback.call(this, this.buffer, this.size);
		});
	});
};

File.write = function(url, data, callback){
	var f = new File(url);
	f.open(function(){
		f.write(data, function(){
			this.close();
			if (typeof callback == "function") callback.call(this);
		});
	});
};


/* -------------------------------------------------------------------------- */

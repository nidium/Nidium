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
	init : function(element){
		print("Native.elements.init()", element);
		var plugin = this[element.type];

		if (!plugin) return false;

		element.__lock("init");


		if (plugin.draw) element.draw = plugin.draw;
		if (plugin.refresh) element.update = plugin.refresh;

		if (plugin.public){
			DOMElement.defineDescriptors(element, plugin.public);
		}

		this.createHardwareLayer(element);
		if (plugin.init){
			print("plugin:init()", element);
			plugin.init.call(element);
		}

		if (element._canReceiveFocus){
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

		element.refresh();
		element.__unlock("init");
		print("-------------------------------------------------------------------------");
	},

	export : function(type, implement){
		if (type=="export" || type=="build" || type=="init") return false;
		this[type] = implement;
		this.build(Native.scope, type);
	},

	/* Native Element Constructor */
	build : function(scope, name){
		scope[String(name)] = function(...n){
			var element = null,
				parent = null,
				className = "",
				options = {};

			switch (n.length) {
				// new Element(parent);
				case 1 :
					parent = n[0];
					options = {};
					break;
					
				// new Element(parent, options);
				case 2 :
					parent = isDOMElement(n[0]) ? n[0] : null;
					options = n[1];

				// new Element();
				default :
					break;					
			}

			if (typeof options == "string") {
				className = options;
				options = {
					class : className
				};
			}

			if (parent == null){
				element = new DOMElement(name, options, null);
			} else {
				if (isDOMElement(parent)){
					element = parent.add(name, options);
				} else {
					throw name + ": Native Element expected";
				}
			}
			return element;
		};
	},

	createHardwareLayer : function(element){
		print("Native.elements.createHardwareLayer("+element._width+", "+element._height+")", element);
		element.layer = new Canvas(element._width, element._height);
		element.layer.padding = element._layerPadding;
		element.layer.context = element.layer.getContext("2D");
		element.layer.host = element;
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

Native.StyleSheet = {
	document : {},

	/* add to the existing stylesheet document */
	add : function(sheet){
		for (var k in sheet){
			if (sheet.hasOwnProperty(k)){
				if (this.document[k]){
					this.mergeProperties(k, sheet[k]);
				} else {
					this.document[k] = sheet[k];
				}

				Native.layout.getElementsByClassName(k).each(function(){
					this.updateProperties();
				});
			}
		}
	},

	/* replace the existing stylesheet document */
	set : function(sheet){
		this.document = {};
		this.add(sheet);
	},

	/* load a local or distant stylesheet (asynchronous) */
	load : function(url){
		var self = this;

		File.getText(url, function(content){
			var sheetText = self.parse(content);
			try {
				eval("self.add(" + sheetText + ")");
			} catch (e) {
				throw ('Error parsing Native StyleSheet "'+url+'"');
			}
		});
	},

	mergeProperties : function(klass, properties){
		var prop = this.document[klass];
		for (var p in properties){
			if (properties.hasOwnProperty(p)){
				prop[p] = properties[p];
			}
		}
	},

	getProperties : function(klass){
		return this.document[klass];
	},

	/*
	 * adapted from James Padolsey's work
	 */
	parse : function(text){
		text = ('__' + text + '__').split('');

		var mode = {
			singleQuote : false,
			doubleQuote : false,
			regex : false,
			blockComment : false,
			lineComment : false
		};

		for (var i = 0, l = text.length; i < l; i++){
			if (mode.regex){
				if (text[i] === '/' && text[i-1] !== '\\'){
					mode.regex = false;
				}
				continue;
			}

			if (mode.singleQuote){
				if (text[i] === "'" && text[i-1] !== '\\'){
					mode.singleQuote = false;
				}
				continue;
			}

			if (mode.doubleQuote){
				if (text[i] === '"' && text[i-1] !== '\\'){
					mode.doubleQuote = false;
				}
				continue;
			}

			if (mode.blockComment){
				if (text[i] === '*' && text[i+1] === '/'){
				text[i+1] = '';
					mode.blockComment = false;
				}
				text[i] = '';
				continue;
			}

			if (mode.lineComment){
				if (text[i+1] === '\n' || text[i+1] === '\r'){
					mode.lineComment = false;
				}
				text[i] = '';
				continue;
			}

			mode.doubleQuote = text[i] === '"';
			mode.singleQuote = text[i] === "'";

			if (text[i] === '/'){
				if (text[i+1] === '*'){
					text[i] = '';
					mode.blockComment = true;
					continue;
				}
				if (text[i+1] === '/'){
					text[i] = '';
					mode.lineComment = true;
					continue;
				}
				mode.regex = true;
			}
		}

		return text.join('')
				.slice(2, -2)
				.replace(/[\n\r]/g, '')
				.replace(/\s+/g, ' ');
	}
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
	f.open("r", function(){
		f.read(f.filesize, function(buffer){
			this.size = buffer.byteLength;
			this.buffer = buffer;
			if (typeof callback == "function") callback.call(this, this.buffer.toString());
		});
	});
};

File.read = function(url, callback){
	var f = new File(url);
	f.open("r", function(){
		f.read(f.filesize, function(buffer){
			this.size = buffer.byteLength;
			this.buffer = buffer;
			if (typeof callback == "function") callback.call(this, this.buffer, this.size);
		});
	});
};

File.write = function(url, data, callback){
	var f = new File(url);
	f.open("w", function(){
		f.write(data, function(){
			this.close();
			if (typeof callback == "function") callback.call(this);
		});
	});
};

File.append = function(url, data, callback){
	var f = new File(url);
	f.open("a", function(){
		f.write(data, function(){
			this.close();
			if (typeof callback == "function") callback.call(this);
		});
	});
};

/* -------------------------------------------------------------------------- */

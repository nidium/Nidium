/* ------------------------+------------- */
/* Native Framework 2.0    | Falcon Build */
/* ------------------------+------------- */
/* (c) 2013 Stight.com - Vincent Fontaine */
/* -------------------------------------- */

"use strict";

/* -------------------------------------------------------------------------- */

Native.canvas.context = Native.canvas.getContext("2D");
Native.scope = this;
Native.scope.ttx = 5;

Object.definePrivateProperties(Native, {
	scope : this,
	elements : {},
	_cachedTextWidth : {},
	blankOrphanedCanvas : new Canvas(1, 1)
});

Object.definePrivateProperties(Native.elements, {
	init : function(element){
		print("Native.elements.init()", element);
		var plugin = this[element.type];

		if (!plugin) return false;

		element.__lock("init");

		if (plugin.draw) element.draw = plugin.draw;
		if (plugin.refresh) element.update = plugin.refresh;
		if (plugin.onAdoption) element.onAdoption = plugin.onAdoption;

		if (plugin.public){
			DOMElement.defineDescriptors(element, plugin.public);
		}

		this.createHardwareLayer(element);
		if (plugin.init){
			print("plugin:init()", element);
			plugin.init.call(element);
		}

		if (element.canReceiveFocus){
			element.addEventListener("mousedown", function(e){
				this.focus();
				e.stopPropagation();
			}, false);
		}

		DOMElement.defineReadOnlyProperties(element, {
			initialized : true
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
		element.layer.context.imageSmoothingEnabled = __ENABLE_IMAGE_INTERPOLATION__;
		element.layer.host = element;
	}
});

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

	/* add to the existing stylesheet document */
	add : function(sheet){
		for (var k in sheet){
			if (sheet.hasOwnProperty(k)){
				this.mergeProperties(k, sheet[k]);
			}
		}
	},

	/* replace the existing stylesheet document */
	set : function(sheet){
		document.stylesheet = {};
		this.add(sheet);
	},

	/* load a local or distant stylesheet */
	load : function(url, sync=true){
		var ____self____ = this,
			sheet = {};

		if (sync) {
			/* the synchronous way uses built-in method load */
			sheet = load(url, 'nss');
			this.add(sheet);
		} else {
			/* the asynchronous way */
			File.getText(url, function(content){
				var sheetText = ____self____.parse(content);
				try {
					eval("____self____.add(" + sheetText + ")");
				} catch (e) {
					throw ('Error parsing Native StyleSheet "'+url+'"');
				}
			});
		}
	},

	/* apply style to all elements with class "klass" */
	updateElements : function(klass){
		Native.layout.getElementsByClassName(klass).each(function(){
			this.updateProperties();
		});
	},

	/* merge properties in an existing class, or create new one */
	mergeProperties : function(klass, properties){
		var prop = document.stylesheet[klass];
		if (prop) {
			for (var p in properties){
				if (properties.hasOwnProperty(p)){
					prop[p] = properties[p];
				}
			}
		} else {
			document.stylesheet[klass] = properties;
		}
		this.updateElements();
	},

	setProperties : function(klass, properties){
		document.stylesheet[klass] = properties;
		this.updateElements();
	},

	getProperties : function(klass){
		return document.stylesheet[klass];
	},

	/*
	 * Native NSS Parser, adapted from James Padolsey's work
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

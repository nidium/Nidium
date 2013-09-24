/* ------------------------+------------- */
/* Native Framework 2.0    | Falcon Build */
/* ------------------------+------------- */
/* (c) 2013 nidium.com - Vincent Fontaine */
/* -------------------------------------- */

"use strict";

/* -------------------------------------------------------------------------- */

Object.definePrivateProperties(Native, {
	elements : {},
	_cachedTextWidth : {},
	blankOrphanedCanvas : new Canvas(1, 1)
});

Object.definePrivateProperties(Native.elements, {
	init : function(element){
		var plugin = this[element.type];

		if (!plugin) return false;

		element.__lock("init");

		if (plugin.draw) element.draw = plugin.draw;
		if (plugin.update) element.update = plugin.update;
		if (plugin.onAdoption) element.onAdoption = plugin.onAdoption;
		if (plugin.onAddChildRequest) element.onAddChildRequest = plugin.onAddChildRequest;
		if (plugin.onChildReady) element.onChildReady = plugin.onChildReady;

		if (plugin.public){
			NDMElement.defineDescriptors(element, plugin.public);
		}

		/* First : apply NSS properties */
		/* maybe we can get width and height properties here */
		element.applyStyleSheet.call(element);

		/* Then : create the physical layer behind (element.layer) */
		this.createCanvasLayer(element);
		
		/* only then, call init() to apply the inline properties */
		if (plugin.init) plugin.init.call(element);

		/* if element can receive focus, add a mousedown listener */
		if (element.canReceiveFocus){
			element.addEventListener("mousedown", function(e){
				this.focus();
				e.stopPropagation();
			}, false);
		}

		/* at this stage, the element is reputed initialized */
		NDMElement.defineReadOnlyProperties(element, {
			initialized : true
		});

		if (typeof(element.onready) == "function"){
			element.onready.call(element);
		}

		element.__refresh();
		element.__unlock("init");
	},

	export : function(type, implement){
		if (type.in("export", "build", "init", "createCanvasLayer")){
			return false;
		}

		this[type] = implement;
		this.build(window.scope, type);
	},

	/* NDMElement Constructor Instanciation */
	build : function(scope, type){
		Object.createProtectedElement(scope, String(type), function(...n){
			var element = null,
				parent = null,
				className = "",
				options = {};

			switch (n.length) {
				case 0 :
					// var element = new UISomething();
					parent = null;
					options = {};
					break;					

				case 1 :
					// var element = new UISomething(parent);
					parent = n[0];
					options = {};
					break;
					
				case 2 :
					// var element = UISomething(parent, options);
					parent = n[0];
					options = n[1];
					break;

				default :
					throw type + "(parent, options) -> maximum 2 parameters";
					break;					
			}

			if (typeof options == "string") {
				className = options;
				options = {
					class : className
				};
			}

			if (parent == null){
				/* create an orphaned element, with no attached parent */
				element = new NDMElement(type, options, null);
			} else {
				/* create a rooted element attached to parent */
				if (isNDMElement(parent)){
					element = parent.add(type, options);
				} else {
					throw type + " -> NDMElement expected";
				}
			}
			return element;
		});
	},

	/* attach the low level canvas layer to element */
	createCanvasLayer : function(element){
		element.layer = new Canvas(element._width, element._height);
		element.layer.padding = element._layerPadding;
		element.layer.context = element.layer.getContext("2D");
		element.layer.context.imageSmoothingEnabled = __ENABLE_IMAGE_INTERPOLATION__;
		element.layer.host = element;
	}
});

/* -------------------------------------------------------------------------- */

Native.getTextWidth = function(text, fontSize, fontFamily){
	var c = Native._cachedTextWidth,
		key = text + fontSize + fontFamily,
		context = Native.blankOrphanedCanvas.getContext("2D");

	if (!c[key]) {
		context.fontSize = fontSize;
		context.fontFamily = fontFamily;
		c[key] = context.measureText(text).width;
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

Native.StyleSheet = {

	/* add to the existing stylesheet document */
	add : function(sheet){
		for (var selector in sheet){
			if (sheet.hasOwnProperty(selector)){
				this.mergeProperties(selector, sheet[selector]);
			}
		}
	},

	/* replace the existing stylesheet document with a new one */
	set : function(sheet){
		document.stylesheet = {};
		this.add(sheet);
	},

	refresh : function(){
		this.add(document.stylesheet);
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

	/* apply style to elements that match selector */
	updateElements : function(selector){
		document.getElementsBySelector(selector).each(function(){
			this.applyStyleSheet();
		});
	},

	/* merge properties in an existing selector, or create new one */
	mergeProperties : function(selector, properties){
		var prop = document.stylesheet[selector];
		if (prop) {
			for (var p in properties){
				if (properties.hasOwnProperty(p)){
					prop[p] = properties[p];
				}
			}
		} else {
			document.stylesheet[selector] = properties;
		}
		this.updateElements(selector);
	},

	setProperties : function(selector, properties){
		document.stylesheet[selector] = properties;
		this.updateElements(selector);
	},

	getProperties : function(selector){
		return document.stylesheet[selector];
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

Object.createProtectedHiddenElement(window.scope, "NDMLayoutParser", {
	parse : function(LST, callback){
		var createElement = function(node, parent){
			var element = null,
				nodeType = node.type,
				nodeAttributes = node.attributes;

			switch (node.type) {
				case "section" :
					nodeType = "UIElement";
					break;

				case "select" :
					nodeType = "UIDropDownController";
					break;

				case "option" :
					nodeType = "UIOption";
					break;

				case "view" :
					nodeType = "UIView";
					break;

				case "button" :
					nodeType = "UIButton";
					break;

				case "slider" :
					nodeType = "UISliderController";
					break;

				case "include":
					nodeType = null;
					break;

				default:
					break;
			}

			if (nodeType) {
				var element = parent.add(nodeType, nodeAttributes);
			}
			return element;
		};

		var parseNodes = function(nodes, parent){
			for (var i=0; i<nodes.length; i++) {
				var node = nodes[i];
				if (node.type != "include") {
					var newParent = createElement(node, parent);
					parseNodes(node.children, newParent);
				}
			}
		};

		parseNodes(LST, document);
		if (typeof callback == "function") callback();
	}
});

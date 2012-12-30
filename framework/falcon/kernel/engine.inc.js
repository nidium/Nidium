/* -------------------------- */
/* Native (@) 2013 Stight.com */
/* -------------------------- */

"use strict";

Native.scope = this;

/* -------------------------------------------------------------------------- */

Native.layout = {
	objID : 0,
	nbObj : 0, // Number of elements

	nodes : {}, // May content several trees of elements
	elements : [], // Flat representation of node trees

	register : function(rootElement){
		this.nodes[rootElement._uid] = rootElement;
	},

	unregister : function(rootElement){
		delete(this.nodes[rootElement._uid]);
	},

	destroy : function(element){
		if (element.parent){
			delete(element.parent.nodes[element._uid]);
			delete(this.nodes[element._uid]);
			element.layer.removeFromParent();
			element = null;
		}
	},

	collectGarbage : function(elements){
		for (var i=elements.length-1; i>0; i--){
			elements[i].__garbageCollector && this.destroy(elements[i]);
		}
	},

	remove : function(rootElement){
		var self = this,
			elements = [];

		this.bubble(rootElement, function(){
			elements.push(this);
			this.__garbageCollector = true;
		});

		this.collectGarbage(elements);
		this.destroy(rootElement);
	},

	/*
	 * Apply Recursive Inference to rootElement and its children
	 */
	bubble : function(rootElement, inference){
		var self = this,
			fn = OptionalCallback(inference, null),
			dx = function(z, parent){
				for (var i in z){
					fn.call(z[i]);
					if (z[i] && self.count(z[i].nodes)>0) {
						/* test z[i] as it may be destroyed by inference */
						dx(z[i].nodes, z[i].parent);
					}
				}
			};

		if (rootElement && fn){
			if (rootElement != this) fn.call(rootElement);
			dx(rootElement.nodes);
		}
	},

	/*
	 * Recompute index on element insertion or removal
	 */
	update : function(){
		var elements = [],
			self = this,
			n = 0;

		this.bubble(this, function(){
			this._nid = n++;
			elements.push(this);
		});

		this.nbObj = n;

		this.elements = elements.sort(function(a, b){
			return a._nid - b._nid;
		});

		this.elements = elements;
	},

	draw : function(){
		var z = this.elements;

		for (var i=0; i<z.length; i++){
			if (z[i]._needRefresh) {
				z[i].refresh();
			}
		}
	},

	find : function(property, value){
		var elements = [],
			z = this.elements;

		for (var i=0; i<z.length; i++){
			let o = z[i];
			if (o[property] && o[property] == value){
				elements.push(o);
			}
		}

		elements.each = function(cb){
			for (var i in elements) {
				if (elements.hasOwnProperty(i) && elements[i]._uid){
					cb.call(elements[i]);
				}
			}
		};

		return elements;
	},

	getElements : function(){
		return this.elements;
	},

	getElementsByName : function(name){
		return this.find("name", name);
	},

	getElementsByClassName : function(name){
		var pattern = new RegExp("(^|\\s)"+name+"(\\s|$)"),
			z = this.elements,
			elements = [];

		for (var i=0; i<z.length; i++){
			pattern.test(z[i].className) && elements.push(z[i]);
		}

		elements.each = function(cb){
			for (var i in elements) {
				if (elements.hasOwnProperty(i) && elements[i]._uid){
					cb.call(elements[i]);
				}
			}
		};

		return elements;
	},

	getElementById : function(id){
		var z = this.elements,
			element = undefined;

		for (var i=0; i<z.length; i++){
			let o = z[i];
			if (o.id && o.id == id){
				element = z[i];
			}
		}
		return element;
	},

	count : function(nodes){
		var len = 0;
		for (var i in nodes){
			if (nodes.hasOwnProperty(i)){
				len++;
			}
		}
		return len;
	}
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

	init : function(element){
		var plugin = this[element.type];

		element.__lock();

		if (element.parent){
			var w = element.getLayerPixelWidth(),
				h = element.getLayerPixelHeight();
			element.layer = new Canvas(w, h);
			element.layer.context = element.layer.getContext("2D");
		} else {
			element._layerPadding = 0;
			element._root = element;

			var w = element.getLayerPixelWidth(),
				h = element.getLayerPixelHeight();
			element.layer = new Canvas(w, h);
			element.layer.context = element.layer.getContext("2D");

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

Native.canvas.context = Native.canvas.getContext("2D");

Native.canvas.implement = function(props){
	Object.merge(CanvasRenderingContext2D.prototype, props);
};

/* -------------------------------------------------------------------------- */

Native._cachedTextWidth = {};
Native.blankOrphanedCanvas = new Canvas(1, 1);
Native.getTextWidth = function(text, fontSize, fontType){
	var c = Native._cachedTextWidth,
		key = text + fontSize + fontType,
		context = Native.blankOrphanedCanvas.getContext("2D");

	//if (!c[key]) {
		context.fontSize = fontSize;
		context.fontType = fontType;
		c[key] = context.measureText(text);
	//}

	return c[key];
};

/* -------------------------------------------------------------------------- */

Native.getLocalImage = function(element, url, callback){
	var cb = OptionalCallback(callback, function(){});
	if (element._cachedBackgroundImage) {
		cb(element._cachedBackgroundImage);
	} else {
		var img = new Image();
		img.onload = function(){
			element._cachedBackgroundImage = img;
			cb(img);
		};
		img.src = url;
	}
};

/* -------------------------------------------------------------------------- */

Native.StyleSheet = {
	document : {},
	add : function(sheet){
		for (var k in sheet){
			if (sheet.hasOwnProperty(k)){
				if (this.document[k]){
					this.mergeProperties(k, sheet[k]);
				} else {
					this.document[k] = sheet[k];
				}
			}
		}
	},

	mergeProperties : function(klass, properties){
		var prop = this.document[klass];
		for (var p in properties){
			prop[p] = properties[p];
		}
	}
};

/* -------------------------------------------------------------------------- */


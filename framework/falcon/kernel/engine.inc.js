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
	elements : [], // Flat representation of node trees (zIndex sorted elements)

	higherzIndex : 0,

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
			return a._zIndex - b._zIndex;
		});

		this.higherzIndex = elements[elements.length-1] ?
			elements[elements.length-1]._zIndex : 0;

		this.elements = elements;
	},

	draw : function(){
		var z = this.elements;

		for (var i=0; i<z.length; i++){
			if (z[i]._needRedraw) {
				var context = z[i].layer.context;
				z[i].update(); // call element's custom refresh method
				z[i].refresh(); // call generic refresh method
				z[i].draw(context);
				z[i]._needRedraw = false;
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
	},

	onPropertyUpdate : function(e){
		var element = e.element,
			old = e.oldValue,
			value = e.newValue;

		element.fireEvent("change", {
			property : e.property,
			oldValue : e.oldValue,
			newValue : e.newValue
		});

		element.refresh();

		switch (e.property) {
			case "left" :
				break;

			case "top" :
				break;

			case "visible" :
				break;
		};

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

		if (!element.parent){
			element._layerPadding = 0;
			element._root = element;
			element.layer = Native.canvas;
			element.layer.context = Native.canvas.context;
			Native.layout.register(element);
		}

		if (plugin){

			if (plugin.refresh) {
				element.update = plugin.refresh;
				element.update();
			}
			if (plugin.init) plugin.init.call(element);
			if (plugin.draw) element.draw = plugin.draw;
			if (element.canReceiveFocus) {
				element.addEventListener("mousedown", function(e){
					this.focus();
					e.stopPropagation();
				}, false);
			}

		} else {
			element.beforeDraw = function(){};
			element.draw = function(context){};
			element.afterDraw = function(){};
		}

		if (element.parent) {
			var w = Math.round(element._width + 2*element._layerPadding),
				h = Math.round(element._height + 2*element._layerPadding);

			element.layer = new Canvas(w, h);
			element.layer.context = element.layer.getContext("2D");
		}

		if (typeof(element.onReady) == "function"){
			element.onReady.call(element);
		}

	}
};

/* -------------------------------------------------------------------------- */

Native.canvas.context = Native.canvas.getContext("2D");

Native.canvas.implement = function(props){
	for (var key in props){
		if (props.hasOwnProperty(key)){
			CanvasRenderingContext2D.prototype[key] = props[key];
		}
	}
};

/* -------------------------------------------------------------------------- */

Native._cachedMeasures = {};
Native.getTextWidth = function(text, fontSize, fontType){
	var c = Native._cachedMeasures,
		key = text + fontSize + fontType,
		canvas = new Canvas(1, 1),
		context = canvas.getContext("2D");

	context.fontSize = fontSize;
	context.fontType = fontType;

	return c[key] ? c[key] : c[key] = context.measureText(text);
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


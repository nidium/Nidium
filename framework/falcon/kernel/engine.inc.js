/* -------------------------- */
/* Native (@) 2013 Stight.com */
/* -------------------------- */

"use strict";

Native.scope = this;

/* -------------------------------------------------------------------------- */

Native.layout = {
	objID : 0,
	nodes : {}, // May content several trees of elements
	elements : [], // Flat representation of Trees (zIndex sorted elements)

	rootElement : null,
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

	draw : function(){

		var z = this.getElements();

		for (var i=0; i<z.length; i++){
			if (z[i].__needRedraw) {
echo(z[i].beforeDraw)

				z[i].beforeDraw();
				z[i].draw();
				z[i].afterDraw();
				z[i].__needRedraw = false;
			}
		}

/*
		if (this.ready===false){
			this.ready = true;
			this.rootElement.fireEvent("load");
		}
*/


	},

	getElements : function(){
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

		return elements;
	},

	find : function(property, value){
		var elements = [],
			self = this;

		this.bubble(this, function(){
			if (this[property] && this[property] == value){
				elements.push(this);
			}
		});

		elements.each = function(cb){
			for (var i in elements) {
				if (elements.hasOwnProperty(i) && elements[i]._uid){
					cb.call(elements[i]);
				}
			}
		};

		return elements;
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
		var self = this,
			plugin = this[element.type];

		if (plugin){

			if (plugin.init) plugin.init.call(element);
			if (plugin.draw) element.draw = plugin.draw;
			if (plugin.isPointInside) {
				element.isPointInside = plugin.isPointInside;
			}
			if (plugin.__construct) element.__construct = plugin.__construct;

			if (element.canReceiveFocus) {
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

Native.getLocalImage = function(element, url, callback){
	var cb = OptionalCallback(callback, function(){});

	if (element._backgroundImage) {
		cb(element._backgroundImage);
	} else {
		var img = new Image();
		img.onload = function(){
			element._backgroundImage = img;
			cb(img);
		};
		img.src = url;
	}
};


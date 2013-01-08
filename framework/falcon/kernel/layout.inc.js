/* ------------------------+------------- */
/* Native Framework 2.0    | Falcon Build */
/* ------------------------+------------- */
/* (c) 2013 Stight.com - Vincent Fontaine */
/* -------------------------------------- */

"use strict";

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

	draw : function(){
		var z = this.elements;

		for (var i=0; i<z.length; i++){
			if (z[i].isVisible() && z[i]._needRefresh) {
				z[i].refresh();
			}
		}

		if (!document.ready){
			document.ready = true;
			document.fireEvent("load");
		}
	},

	/*
	 * Rebuild index on element insertion or removal
	 */
	update : function(){
		var elements = [],
			self = this,
			n = 0;

		this.bubble(this, function _update_bubbleCallback(){
			this._nid = n++;
			elements.push(this);
		});

		this.nbObj = n;

		this.elements = elements.sort(function _update_sort(a, b){
			return a._nid - b._nid;
		});

		this.elements = elements;
	},

	setContentSize : function(element){
		var mx = 0,
			my = 0;

		/* The idea here is to compute contentWidth and contentHeigh
		 * of element. To do that, we need to recursively parse all its
		 * children to find the "farest" one from the left top corner of
		 * our element.
		 */

//		var e = "";
		this.bubble(element, function(){
			//echo(e, this.id, this._absx, this._absy);
			//e+="    ";
			// exlude element itself
			if (this == element) return false;

			var	x1 = this._absx - element._absx,
				y1 = this._absy - element._absy,
				x2 = x1 + this._width,
				y2 = y1 + this._height;

			if (x2>mx) mx = x2;
			if (y2>my) my = y2;
		});

		//element.__lock();
		element.contentWidth = mx;
		element.contentHeight = my;
		//element.__unlock();
	},

	updateInnerContentTree : function(element){
		if (!element.parent) return false;
		while (element.parent){
			this.setContentSize(element.parent);
			element = element.parent;
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
				if (isDOMElement(elements[i])){
					cb.call(elements[i]);
				}
			}
		};

		return elements;
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

	getElements : function(){
		return this.elements;
	},

	getElementsByName : function(name){
		return this.find("name", name);
	},

	getElementsByTagName : function(name){
		return this.find("type", name);
	},

	getElementsByClassName : function(className){
		var pattern = new RegExp("(^|\\s)"+className+"(\\s|$)"),
			z = this.elements,
			elements = [];

		for (var i=0; i<z.length; i++){
			pattern.test(z[i]._className) && elements.push(z[i]);
		}

		elements.each = function(cb){
			if (typeof cb != "function") return false;
			for (var i in elements) {
				if (isDOMElement(elements[i])){
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
	}
};

/* ---------------------------------------------------------------------- */

Object.createProtectedElement(Native.scope, "Application", function(options){
	options = options || {};
	options.canReceiveFocus = true;
	options.outlineOnFocus = false;

	var element = new DOMElement("UIView", options, null);
	Native.layout.update();

	return element;
});

Object.createProtectedElement(Native.scope, "document", new Application({
	left : 0,
	top : 0,
	width : window.width,
	height : window.height,
	background : "#262722",
	canReceiveFocus : true,
	outlineOnFocus : false
}));

/* -------------------------------------------------------------------------- */

window.requestAnimationFrame(function(){
	Native.FPS.start();
	Native.layout.draw();
	Native.FPS.show();
});

/* -------------------------------------------------------------------------- */

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
	focusID : 0,

	nodes : [], // May content several trees of elements
	elements : [], // Flat representation of node trees

	init : function(element){
		Native.elements.init(element);

		if (element._className != '') {
			element.updateProperties();
		}

		if (element.parent) {
			element.parent.layer.add(element.layer);
		} else {
			Native.canvas.add(element.layer);
		}

	},

	register : function(rootElement){
		this.nodes.push(rootElement);
	},

	unregister : function(rootElement){
		for (var i=0; i<this.nodes.length; i++){
			if (this.nodes[i] == rootElement) {
				this.nodes.splice(i, 1);
				break;
			}
		}
	},

	draw : function(){
		var z = this.elements;

		for (var i=0; i<z.length; i++){
			var element = z[i];
			if (element.hasOwnerDocument){
				if (element._needRefresh){
					element.refresh();
				}
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
				for (var i=0; i<z.length; i++){
					fn.call(z[i]);
					if (z[i] && z[i].nodes.length>0) {
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

	getElementUnderPointer : function(){
		var element = null,
			x = window.mouseX,
			y = window.mouseY,
			z = this.elements;

		for (var i=z.length-1 ; i>=0 ; i--) {
			if (z[i].layer.__visible && z[i].isPointInside(x, y)) {
				element = z[i];
				break;
			}
		}
		return element;
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

	focusNextElement : function(){
		var self = this,
			z = this.elements;

		this.focusID++;
		if (this.focusID > this.nbObj-2) {
			this.focusID = 0;
		}

		for (var i=z.length-1; i>=0; i--){
			var element = z[i];
			if (this.focusID == element._nid){
				if (element.canReceiveFocus){
					this.focus(element);
					break;
				} else {
					this.focusNextElement();
				}
			}
		}
	},

	focus : function(element){
		if (element.hasFocus === true) {
			return false;
		}
		if (element.canReceiveFocus) {
			/* Fire blur event on last focused element */
			if (this.currentFocusedElement) {
				if (this.currentFocusedElement.outlineOnFocus) {
					this.currentFocusedElement.outline = false;
				}
				this.currentFocusedElement.fireEvent("blur", {});
				this.currentFocusedElement.hasFocus = false;
			}

			/* set this element as the new focused element */
			element.hasFocus = true;
			if (element.outlineOnFocus) {
				element.outline = true;
			}
			element.fireEvent("focus", {});
			this.currentFocusedElement = element;
			this.focusID = element._nid;
		}
	},

	remove : function(element){
		var parent = element.parent;
		if (!parent) return false;

		element.layer.removeFromParent();
		element.resetNodes();
		/*
		for (var p in element){
			delete element[p];
		}
		*/
	}
};

/* ---------------------------------------------------------------------- */

Object.createProtectedElement(Native.scope, "Application", function(options){
	options = options || {};
	options.canReceiveFocus = true;
	options.outlineOnFocus = false;

	var element = new DOMElement("UIView", options, null);
	element._root = element;

	Native.layout.init(element, null);
	Native.layout.register(element);
	Native.layout.update();

	return element;
});

/* -------------------------------------------------------------------------- */

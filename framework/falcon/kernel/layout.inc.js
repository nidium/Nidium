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

	nodes : [], // May content several trees of elements
	elements : [], // Flat representation of node trees

	register : function(rootElement){
		this.nodes.push(rootElement);
	},

	unregister : function(rootElement){
		/* TODO
		delete(this.nodes[rootElement._uid]);
		*/
	},

	draw : function(){
		var z = this.elements;

		for (var i=0; i<z.length; i++){
			if (z[i]._needRefresh){
				z[i].refresh();
			} else {
				z[i].__left = z[i].layer.__left;
				z[i].__top = z[i].layer.__top;
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

	remove : function(element){
		var parent = element.parent;
		if (!parent) return false;

		element.layer.removeFromParent();
		element.resetNodes();

		for (var p in element){
			delete element[p];
		}
	}
};

/* ---------------------------------------------------------------------- */

Object.createProtectedElement(Native.scope, "Application", function(options){
	options = options || {};
	options.canReceiveFocus = true;
	options.outlineOnFocus = false;

	var element = new DOMElement("UIView", options, null);

	Native.canvas.add(element.layer);
	Native.layout.register(element);
	Native.layout.update();

	return element;
});

Object.createProtectedElement(Native.scope, "document", new Application({
	id : "document",
	left : 0,
	top : 0,
	width : window.width,
	height : window.height,
	background : "#262722",
	canReceiveFocus : true,
	outlineOnFocus : false
}));
document.__styleSheetLoaded = true;

/* -------------------------------------------------------------------------- */

window.requestAnimationFrame(function(){
	Native.FPS.start();
	Native.layout.draw();
	if (Native.layout.drawHook) Native.layout.drawHook();
	Native.FPS.show();
});

/* -------------------------------------------------------------------------- */

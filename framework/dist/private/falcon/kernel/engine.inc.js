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
		if (plugin.resize) element.resize = plugin.resize;
		if (plugin.onAdoption) element.onAdoption = plugin.onAdoption;
		if (plugin.onAddChildRequest) element.onAddChildRequest = plugin.onAddChildRequest;
		if (plugin.onChildReady) element.onChildReady = plugin.onChildReady;

		if (plugin.public){
			NDMElement.definePublicDescriptors(element, plugin.public);
		}

		/* First : apply NSS properties */
		/* maybe we can get width and height properties here */
		element.applyStyleSheet.call(element);

		/* Only then, apply the inline style properties to the element */
		element.applyInlineProperties.call(element);

		/* Then : create the physical layer behind (element.layer) */
		this.createCanvasLayer(element);
		
		/* Then : call the plugin's init() method */
		if (plugin.___source___ && plugin.___source___.init) plugin.___source___.init.call(element);
		if (plugin.init) plugin.init.call(element);

		/* Then : call the element's resize() method */
		element.resize.call(element);

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
		var self = this;
		if (type.in("extend", "export", "build", "init", "createCanvasLayer")){
			return false;
		}

		var superize = function(t, key, fn){
			var that = this,
				element = self[t],
				__super__ = element[key];

			if (key == "ffinit") {
				element.init = function(){
					//__super__.call(that);
					fn();
				}.bind(this);
			} else {
				element[key] = fn;
			}
		};

		if (this.___source___) {
			/* build from an existing type */

			this[type] = {};
			this[type].___source___ = this.___source___;

			// copy all from source
			for (var i in this.___source___){
				if (this.___source___.hasOwnProperty(i)) {
					this[type][i] = this.___source___[i];
				}
			}

			console.log("");
			for (var key in implement){
				if (implement.hasOwnProperty(key)) {
					var value = implement[key];

					this[type][key] = value;
					/*
					if (typeof(value) == "function") {
						superize(type, key, value);
					} else {
						// overwrite properties
						this[type][key] = value;
					}
					*/
					
				}
			}

			this.___source___ = null;
		} else {
			/* new creation */
			this[type] = implement;
		}

		this.build(window.scope, type);
	},

	extend : function(type){
		this.___source___ = this[type];
		if (type.in("extend", "export", "build", "init", "createCanvasLayer")){
			return false;
		}
		if (!this.___source___) throw "Unable to extend undefined " + type;

		return this;
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
		/* FIX ME : CRASH if w or h = NaN or 0 or undefined */
		var w = element._width || 1,
			h = element._height || 1;

		element.layer = new Canvas(w, h);
		element.layer.padding = element._layerPadding;
		element.layer.context = element.layer.getContext("2d");
		element.layer.context.imageSmoothingEnabled = __ENABLE_IMAGE_INTERPOLATION__;
		element.layer.host = element;

		/* attach the element to its parent */
		if (element.parent) {
			element.parent.layer.add(element.layer);
		} else {
			window.canvas.add(element.layer);
		}
	}
});

/* -------------------------------------------------------------------------- */

Native.getTextWidth = function(text, fontSize, fontFamily){
	var c = Native._cachedTextWidth,
		key = text + fontSize + fontFamily,
		context = Native.blankOrphanedCanvas.getContext("2d");

	if (!c[key]) {
		context.fontSize = fontSize;
		context.fontFamily = fontFamily;
		c[key] = context.measureText(text).width;
	}
	return c[key];
};

/* -------------------------------------------------------------------------- */
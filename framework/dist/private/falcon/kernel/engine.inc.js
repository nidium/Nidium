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
			NDMElement.definePublicDescriptors(element, plugin.public);
		}

		/* First : apply NSS properties */
		/* maybe we can get width and height properties here */
		element.applyStyleSheet.call(element);

		/* Only then, apply the inline style properties to the element */
		element.applyInlineProperties.call(element);

		/* Then : create the physical layer behind (element.layer) */
		this.createCanvasLayer(element);
		
		/* Then : call the plugin init() method */
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
		/* FIX ME : CRASH if NaN or 0 or undefined */
		element._width = element._width || 1;
		element._height = element._height || 1;
		/* FIX ME : CRASH if NaN or 0 or undefined */

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
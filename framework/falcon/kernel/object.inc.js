/* ------------------------+------------- */
/* Native Framework 2.0    | Falcon Build */
/* ------------------------+------------- */
/* (c) 2013 Stight.com - Vincent Fontaine */
/* -------------------------------------- */

"use strict";

/* -------------------------------------------------------------------------- */

var DOMElement = function(type, options, parent){
	var o = this.options = options || {},
		p = this.parent = parent ? parent : null; // parent element

	if (!Native.elements[type]) {
		throw("Undefined element " + type);
	}

	if (o && o.class) {
		o.className = o.class;
	}

	this.parent = p;
	this.nodes = {}; // children elements

	/* Read Only Properties */
	DOMElement.defineReadOnlyProperties(this, {
		type : type,
		isDOMElement : true
	});

	/* Public Dynamic Properties (visual impact on element, need redraw) */
	/* Common to all elements */
	DOMElement.definePublicProperties(this, {
		// -- class management
		className : OptionalString(o.class, ""),

		// -- layout properties
		left : OptionalNumber(o.left, 0),
		top : OptionalNumber(o.top, 0),
		width : o.width ? Number(o.width) : p ? p._width : window.width,
		height : o.height ? Number(o.height) : p ? p._height : window.height,

		contentWidth : 0,
		contentHeight : 0,

		scrollLeft : OptionalNumber(o.scrollLeft, 0),
		scrollTop : OptionalNumber(o.scrollTop, 0),

		percentLeft : OptionalNumber(o.percentLeft, 0),
		percentTop : OptionalNumber(o.percentTop, 0),
		percentWidth : o.percentWidth ? Number(o.percentWidth) : 100,
		percentHeight : o.percentHeight ? Number(o.percentHeight) : 100,

		offsetLeft : OptionalNumber(o.offsetLeft, 0),
		offsetTop : OptionalNumber(o.offsetTop, 0),

		paddingLeft : OptionalNumber(o.paddingLeft, 0),
		paddingRight : OptionalNumber(o.paddingLeft, 0),

		// -- text related properties
		label : OptionalString(o.label, ""),
		fontSize : OptionalNumber(o.fontSize, 12),
		fontType : OptionalString(o.fontType, "arial"),
		textAlign : OptionalAlign(o.textAlign, "left"),
		lineHeight : OptionalNumber(o.lineHeight, 18),

		// -- style properties
		blur : OptionalNumber(o.blur, 0),
		opacity : OptionalNumber(o.opacity, 1),
		alpha : OptionalNumber(o.alpha, 1),

		shadowOffsetX : OptionalNumber(o.shadowOffsetX, 0),
		shadowOffsetY : OptionalNumber(o.shadowOffsetY, 0),
		shadowBlur : OptionalNumber(o.shadowBlur, 0),
		shadowColor : OptionalValue(o.shadowColor, "rgba(0, 0, 0, 0.5)"),

		textShadowOffsetX : OptionalNumber(o.textShadowOffsetX, 0),
		textShadowOffsetY : OptionalNumber(o.textShadowOffsetY, 0),
		textShadowBlur : OptionalNumber(o.textShadowBlur, 0),
		textShadowColor : OptionalValue(o.textShadowColor, ''),

		color : OptionalValue(o.color, ''),
		background : OptionalValue(o.background, ''),
		backgroundImage : OptionalValue(o.backgroundImage, ''),
		radius : OptionalNumber(o.radius, 0, 0),

		angle : OptionalNumber(o.angle, 0),
		scale : OptionalNumber(o.scale, 1),

		// -- misc flags
		canReceiveFocus : OptionalBoolean(o.canReceiveFocus, false),
		outlineOnFocus : OptionalBoolean(o.outlineOnFocus, false),

		visible : OptionalBoolean(o.visible, true),
		selected : OptionalBoolean(o.selected, false),
		overflow : OptionalBoolean(o.overflow, true),
		scrollbars : OptionalBoolean(o.scrollbars, false),
		position : OptionalPosition(o.position, "relative"),

		hover : false,
		hasFocus : false
	});

	/* Internal Hidden Properties */
	DOMElement.defineInternalProperties(this, {
		private : {},
		_root : p ? p._root : this,
		_nid : Native.layout.objID++,
		_uid : "_obj_" + Native.layout.objID,
		_eventQueues : [],
		_mutex : [],
		_locked : true,

		_cachedBackgroundImage : null,

		_minx : this._left,
		_miny : this._top,
		_maxx : this._left + this._width,
		_maxy : this._top + this._top,

		_layerPadding : p ? Math.max(this.shadowBlur*4, 20) : Math.max(this.shadowBlur*4, 10),

		/* absolute value (inherited) */
		__left : 0,
		__top : 0,

		/* refreshing flags */
		_needRefresh : true,
		_needRedraw : true,
		_needPositionUpdate : true,
		_needSizeUpdate : true,
		_needOpacityUpdate : true,
		_needAncestorCacheClear : false
	});

	/* Runtime changes does not impact the visual aspect of the element */
	this.id = OptionalString(o.id, this._uid);
	this.name = OptionalString(o.name, "");
	this.isOnTop = false;
	this.hasChildren = false;
	this.mouseOverPath = false;

	if (p){
		p.hasChildren = true;
	}

	print("DOMElement.constructor", this);
	Native.elements.init(this);

	if (this._className != '') {
		this.updateProperties();
	}
};

/* -------------------------------------------------------------------------- */

DOMElement.prototype = {
//	__noSuchMethod__ : Native.object.__noSuchMethod__,

	__lock : Native.object.__lock, // disable setter events
	__unlock : Native.object.__unlock, // enable setter events

	add : Native.object.add,
	remove : Native.object.remove,
	show : Native.object.show,
	hide : Native.object.hide,
	focus : Native.object.focus,

	addChild : Native.object.addChild,
	removeChild : Native.object.removeChild,
	getChildren : Native.object.getChildren,

	refresh : Native.object.refresh,
	
	beforeDraw : Native.object.beforeDraw,
	afterDraw : Native.object.afterDraw,

	isPointInside : Native.object.isPointInside,
	isVisible : Native.object.isVisible,
	getDrawingBounds : Native.object.getDrawingBounds,
	getBoundingRect : Native.object.getBoundingRect,

	hasClass : Native.object.hasClass,
	addClass : Native.object.addClass,
	removeClass : Native.object.removeClass,

	bringToFront : Native.object.bringToFront,
	sendToBack : Native.object.sendToBack,
	resetNodes : Native.object.resetNodes,

	updateProperties : Native.object.updateProperties,
	setProperties : Native.object.setProperties,

	center : Native.object.center,
	centerLeft : Native.object.centerLeft,
	centerTop : Native.object.centerTop,
	move : Native.object.move,
	fix : Native.object.fix,

	redraw : Native.object.redraw,

	updateLayerOpacity : Native.object.updateLayerOpacity,
	updateLayerPosition : Native.object.updateLayerPosition,
	updateLayerSize : Native.object.updateLayerSize,
	updateAncestors : Native.object.updateAncestors,
	resizeLayer : Native.object.resizeLayer,
	expand : Native.object.expand,

	/* -- READ ONLY WRAPPERS -- */

	get children() {
		return this.getChildren();
	},

	get previousSibling() {
		return this.layer.getPrevSibling().host;
	},

	get nextSibling() {
		return this.layer.getNextSibling().host;
	},

	get childNodes() {
		var nodes = [];
		for (var i in this.nodes){
			if (isDOMElement(this.nodes[i])) nodes.push(this.nodes[i]);
		}
		return nodes;
	},

	get parentNode(){
		return this.parent ? this.parent : null;
	},

	get nodeType() {
		return this.type;
	},

	get nodeName() {
		return this.name;
	},

	get ownerDocument() {
		return this._root;
	},

	/* -- user customisable methods -- */

	update : function(context){},
	draw : function(context){}
};

/* -------------------------------------------------------------------------- */

DOMElement.onPropertyUpdate = function(e){
	var element = e.element,
		old = e.oldValue,
		value = e.newValue;

	print("DOMElement.onPropertyUpdate("+e.property+")", element);

	element.__unlock();

	element.fireEvent("change", {
		property : e.property,
		oldValue : e.oldValue,
		newValue : e.newValue
	});

	element.__lock("onPropertyUpdate");

	switch (e.property) {
		case "left" :
		case "top" :
			element._needPositionUpdate = true;
			element._needAncestorCacheClear = true;
			break;

		case "scrollLeft" :
		case "scrollTop" :
			element._needPositionUpdate = true;
			break;

		case "width" :
		case "height" :
			element._needSizeUpdate = true;
			element._needAncestorCacheClear = true;
			element._needRedraw = true;
			break;

		case "opacity" :
			element._needOpacityUpdate = true;
			break;

		case "className" :
			element.updateProperties();
			element._needRedraw = true;
			break;

		case "position" :
			break;

		case "angle" :
			element.resizeLayer();
			break;

		default :
			element._needRedraw = true;
			break
	};

	element._needRefresh = true;
	element.__unlock("onPropertyUpdate");
};

/* -------------------------------------------------------------------------- */

DOMElement.implement = function(props){
	Object.merge(Native.object, props);
	for (var key in props){
		if (props.hasOwnProperty(key)){
			DOMElement.prototype[key] = Native.object[key];
		}
	}
};

var	isDOMElement = function(element){
	return element && element.isDOMElement;
};

/* -------------------------------------------------------------------------- */

DOMElement.listeners = {
	addDefault : function(element){
		this.addSelectors(element);
		this.addHovers(element);
	},

	addSelectors : function(element){
		element.addEventListener("mousedown", function(e){
			this.selected = true;
		});

		element.addEventListener("mouseup", function(e){
			this.selected = false;
		});

		element.addEventListener("dragend", function(e){
			this.selected = false;
		});
	},

	addHovers : function(element){
		element.addEventListener("mouseover", function(e){
			this.hover = true;
		});

		element.addEventListener("mouseout", function(e){
			this.hover = false;
		});
	}
};

/* -------------------------------------------------------------------------- */

DOMElement.draw = {
	circleBackground : function(element, context, params, radius){
		var gradient = context.createRadialGradient(
			params.x+radius,
			params.y+radius, 
			radius,
			params.x+radius,
			params.y+radius,
			radius/4
		);

		if (element.hover){
			gradient.addColorStop(0.00, 'rgba(0, 0, 0, 0.01)');
			gradient.addColorStop(1.00, 'rgba(0, 0, 0, 0.1)');
		} else {
			gradient.addColorStop(0.00, 'rgba(255, 255, 255, 0.01)');
			gradient.addColorStop(1.00, 'rgba(255, 255, 255, 0.1)');
		}

		context.beginPath();
		context.arc(
			params.x+radius, params.y+params.h*0.5, 
			radius, 0, 6.2831852, false
		);
		context.setColor(element.background);
		context.fill();
		context.lineWidth = 1;

		context.beginPath();
		context.arc(
			params.x+radius, params.y+params.h*0.5, 
			radius, 0, 6.2831852, false
		);
		context.setColor(gradient);
		context.fill();
		context.lineWidth = 1;

		if (element.selected){
			context.beginPath();
			context.arc(
				params.x+radius, params.y+params.h*0.5, 
				radius, 0, 6.2831852, false
			);
			context.setColor(element.background);
			context.fill();
			context.lineWidth = 1;
		}
	},

	label : function(element, context, params, color, shadowColor){
		var textOffsetX = element.paddingLeft,
			textOffsetY = (params.h-element.lineHeight)/2 + 4 + element.lineHeight/2;

		var tx = params.x+textOffsetX,
			ty = params.y+textOffsetY;

		if (element.textAlign == "right") {
			tx = params.x + params.w - element._textWidth - element.paddingRight;
		}

		context.setFontSize(element.fontSize);
		context.setFontType(element.fontType);

		context.setText(
			element.label,
			params.x+textOffsetX+params.textOffsetX,
			params.y+textOffsetY+params.textOffsetY,
			color ? color : element.color,
			element.textShadowOffsetX,
			element.textShadowOffsetY,
			element.textShadowBlur,
			shadowColor ? shadowColor : element.textShadowColor
		);

	},

	box : function(element, context, params, backgroundColor){
		context.roundbox(
			params.x, params.y, 
			params.w, params.h, 
			element.radius,
			backgroundColor ? backgroundColor : element.background,
			false
		);
	},

	glassLayer : function(element, context, params){
		var gradient = this.getGlassGradient(element, context, params);
		this.box(element, context, params, gradient);
	},

	getGlassGradient : function(element, context, params){
		var gradient = context.createLinearGradient(
			params.x, params.y,
			params.x, params.y+params.h
		);

		if (element.selected){
			gradient.addColorStop(0.00, 'rgba(0, 0, 0, 0.8)');
			gradient.addColorStop(0.25, 'rgba(0, 0, 0, 0.6)');
			gradient.addColorStop(1.00, 'rgba(0, 0, 0, 0.6)');
		} else {
			if (element.hover){
				gradient.addColorStop(0.00, 'rgba(255, 255, 255, 0.7)');
				gradient.addColorStop(0.50, 'rgba(255, 255, 255, 0.3)');
				gradient.addColorStop(0.50, 'rgba(255, 255, 255, 0.1)');
				gradient.addColorStop(0.80, 'rgba(0, 0, 0, 0.1)');
				gradient.addColorStop(1.00, 'rgba(0, 0, 0, 0.3)');
			} else {
				gradient.addColorStop(0.00, 'rgba(255, 255, 255, 0.4)');
				gradient.addColorStop(0.50, 'rgba(255, 255, 255, 0.1)');
				gradient.addColorStop(0.50, 'rgba(255, 255, 255, 0.0)');
				gradient.addColorStop(0.50, 'rgba(0, 0, 0, 0.0)');
				gradient.addColorStop(0.80, 'rgba(0, 0, 0, 0.1)');
				gradient.addColorStop(1.00, 'rgba(0, 0, 0, 0.3)');
			}
		}
		return gradient;
	},

	getSmoothGradient : function(element, context, params){
		var gradient = context.createLinearGradient(
			params.x, params.y,
			params.x, params.y+params.h
		);

		gradient.addColorStop(0.00, 'rgba(255, 255, 255, 0.08)');
		gradient.addColorStop(1.00, 'rgba(0, 0, 0, 0.25)');

		return gradient;
	},

	getSoftGradient : function(element, context, params){
		var gradient = context.createLinearGradient(
			params.x, params.y, 
			params.x, params.y+params.h
		);

		if (element.selected){
			gradient.addColorStop(0.00, 'rgba(255, 255, 255, 0.20)');
			gradient.addColorStop(0.10, 'rgba(255, 255, 255, 0.05)');
			gradient.addColorStop(0.90, 'rgba(255, 255, 255, 0.00)');

		} else {
			if (element.hover){
				gradient.addColorStop(0.00, 'rgba(255, 255, 255, 0.30)');
				gradient.addColorStop(0.25, 'rgba(255, 255, 255, 0.18)');
			} else {
				gradient.addColorStop(0.00, 'rgba(255, 255, 255, 0.25)');
				gradient.addColorStop(0.10, 'rgba(255, 255, 255, 0.15)');
			}

		}
		return gradient;
	},

	getCleanGradient : function(element, context, params){
		var gradient = context.createLinearGradient(
			params.x, params.y, 
			params.x, params.y+params.h
		);

		gradient.addColorStop(0.00, 'rgba(255, 255, 255, 0.20)');
		gradient.addColorStop(0.10, 'rgba(255, 255, 255, 0.05)');
		gradient.addColorStop(0.90, 'rgba(255, 255, 255, 0.00)');
		return gradient;
	},

	getInnerTextWidth : function(element){
		var w = Native.getTextWidth(
			element._label,
			element._fontSize,
			element._fontType
		);
		return element._paddingLeft + Math.round(w) + element._paddingRight;
	}
};

/* -------------------------------------------------------------------------- */

DOMElement.defineNativeProperty = function(descriptor){
	var element = descriptor.element, // target element
		property = descriptor.property, // property
		value = OptionalValue(descriptor.value, null), // default value
		setter = OptionalCallback(descriptor.setter, null),
		getter = OptionalCallback(descriptor.getter, null),
		writable = OptionalBoolean(descriptor.writable, true),
		enumerable = OptionalBoolean(descriptor.enumerable, true),
		configurable = OptionalBoolean(descriptor.configurable, false);

	if (!element || !property) return false;

	/* if value is undefined, get the previous value */
	if (value == undefined && element["_"+property] && element[property]) {
		value = element["_"+property];
	}

	/* define mirror hidden properties */
	Object.createHiddenElement(element, "_"+property, value);

	/* define public accessor */
	Object.defineProperty(element, property, {
		get : function(){
			var r = undefined;
			if (element._locked === false) {
				print("unlocked get("+property+")", element);
				element.__lock("plugin:"+property);
				r = getter ? getter.call(element) : undefined;
				element.__unlock("plugin:"+property);
			} else {
				print("locked get "+property, element);
			}
			return r == undefined ? element["_"+property] : r;
		},

		set : function(newValue){
			var oldValue = element["_"+property];

			if (newValue === oldValue || writable === false) {
				return false;
			} else {
				element["_"+property] = newValue;
			}

			if (element.loaded && element._locked === false) {
				print("set "+property+' = "'+newValue+'"', element);

				/* lock element */
				element.__lock("plugin:"+property);

				/* fire propertyupdate event if needed */
				DOMElement.onPropertyUpdate({
					element : element,
					property : property,
					oldValue : oldValue,
					newValue : newValue
				});

				/* optional user defined setter method */
				if (setter){
					print("plugin:set("+property+"="+newValue+")", element);
					var r = setter.call(element, newValue);
					if (r === false) {
						// handle readonly, restore old value
						element["_"+property] = oldValue;
						return false;
					}
				}

				/* unlock element */
				element.__unlock("plugin:"+property);
			}

		},

		enumerable : enumerable,
		configurable : configurable
	});
};

/* -------------------------------------------------------------------------- */

DOMElement.defineDescriptors = function(element, props){
	print("DOMElement.defineDescriptors", element);
	for (var key in props){
		if (props.hasOwnProperty(key)){
			var descriptor = props[key],
				value = element["_"+key] != undefined ?
						element["_"+key] : undefined;

			if (descriptor.value){
				if (typeof descriptor.value == "function"){
					value = descriptor.value.call(element);
				} else {
					value = descriptor.value;
				}
			}

			if (descriptor.writable === false){
				if (descriptor.setter || descriptor.getter){
					throw(element.type + '.js : Setter and Getter must not be defined for non writabe property "'+key+'".');
				}			
			}

			this.defineNativeProperty({
				element : element,
				property : key,
				value : value,

				setter : descriptor.set,
				getter : descriptor.get,

				writable : descriptor.writable,
				enumerable : descriptor.enumerable,
				configurable : descriptor.configurable
			});
		}
	}
};

/* -------------------------------------------------------------------------- */

DOMElement.definePublicProperties = function(element, props){
	for (var key in props){
		if (props.hasOwnProperty(key)){
			this.defineNativeProperty({
				element : element,
				property : key,
				value : props[key],
				enumerable : true,
				configurable : true
			});
		}
	}
};

DOMElement.defineReadOnlyProperties = function(element, props){
	for (var key in props){
		if (props.hasOwnProperty(key)){
			Object.createProtectedElement(element, key, props[key]);
		}
	}
};

DOMElement.defineInternalProperties = function(element, props){
	for (var key in props){
		if (props.hasOwnProperty(key)){
			Object.createHiddenElement(element, key, props[key]);
		}
	}
};

/* -------------------------------------------------------------------------- */

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
		/* ------------------------------------------------------------------ */
		/* DO NOT NEED REDRAW PROPERTIES                                      */
		/* ------------------------------------------------------------------
		left, top, width, height,
		percentLeft, percentTop, percentWidth, percentHeight
		--------------------------------------------------------------------- */ 

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

		shadowOffsetX : OptionalNumber(o.shadowOffsetX, 0),
		shadowOffsetY : OptionalNumber(o.shadowOffsetY, 0),
		shadowBlur : OptionalNumber(o.shadowBlur, 0),
		shadowColor : OptionalNumber(o.shadowColor, "rgba(0, 0, 0, 0.5)"),

		textShadowOffsetX : OptionalNumber(o.textShadowOffsetX, 0),
		textShadowOffsetY : OptionalNumber(o.textShadowOffsetY, 0),
		textShadowBlur : OptionalNumber(o.textShadowBlur, 0),
		textShadowColor : OptionalNumber(o.textShadowColor, "rgba(0, 0, 0, 0.4)"),

		color : OptionalValue(o.color, ''),
		background : OptionalValue(o.background, ''),
		backgroundImage : OptionalValue(o.backgroundImage, ''),
		radius : OptionalNumber(o.radius, 0, 0),

		// -- misc flags
		canReceiveFocus : OptionalBoolean(o.canReceiveFocus, false),
		outlineOnFocus : OptionalBoolean(o.outlineOnFocus, false),

		visible : OptionalBoolean(o.visible, true),
		selected : OptionalBoolean(o.selected, false),
		overflow : OptionalBoolean(o.overflow, true),
		fixed : OptionalBoolean(o.fixed, false),

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

		_cachedContentWidth : null,
		_cachedContentHeight : null,
		_cachedBackgroundImage : null,

		_minx : this._left,
		_miny : this._top,
		_maxx : this._left + this._width,
		_maxy : this._top + this._top,

		_layerPadding : p ? 20 : 0,

		/* absolute value (inherited) */
		__left : 0,
		__top : 0,
		__opacity : this._opacity,
		__scrollTop : this._scrollTop,
		__overflow : this._overflow,
		__fixed : this._fixed,
		__layerPadding : 20,

		/* refreshing flags */
		_needRefresh : true,
		_needRedraw : true,
		_needPositionUpdate : true,
		_needSizeUpdate : true,
		_needOpacityUpdate : true,
		_needContentSizeUpdate : true
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
	__noSuchMethod__ : Native.object.__noSuchMethod__,

	__lock : Native.object.__lock, // disable setter events
	__unlock : Native.object.__unlock, // enable setter events

	updateInheritance : Native.object.updateInheritance,

	add : Native.object.add,
	remove : Native.object.remove,
	show : Native.object.show,
	hide : Native.object.hide,
	focus : Native.object.focus,

	addChild : Native.object.addChild,
	removeChild : Native.object.removeChild,

	refresh : Native.object.refresh,
	
	beforeDraw : Native.object.beforeDraw,
	afterDraw : Native.object.afterDraw,

	isPointInside : Native.object.isPointInside,
	isVisible : Native.object.isVisible,
	getDrawingBounds : Native.object.getDrawingBounds,

	hasClass : Native.object.hasClass,
	addClass : Native.object.addClass,
	removeClass : Native.object.removeClass,

	getLayerPixelWidth : Native.object.getLayerPixelWidth,
	getLayerPixelHeight : Native.object.getLayerPixelHeight,

	bringToFront : Native.object.bringToFront,
	sendToBack : Native.object.sendToBack,
	resetNodes : Native.object.resetNodes,

	updateProperties : Native.object.updateProperties,
	setProperties : Native.object.setProperties,

	redraw : Native.object.redraw,

	updateLayerOpacity : Native.object.updateLayerOpacity,
	updateLayerPosition : Native.object.updateLayerPosition,
	updateLayerSize : Native.object.updateLayerSize,
	updateAncestors : Native.object.updateAncestors,

	/* user customisable methods */
	update : function(context){},
	draw : function(context){}
};

/* -------------------------------------------------------------------------- */

DOMElement.onPropertyUpdate = function(e){
	var element = e.element,
		old = e.oldValue,
		value = e.newValue;

	print("DOMElement.onPropertyUpdate("+e.property+")", element);

	element.__lock("DOM");

	element.fireEvent("change", {
		property : e.property,
		oldValue : e.oldValue,
		newValue : e.newValue
	});

	switch (e.property) {
		case "left" :
		case "top" :
			element._needPositionUpdate = true;
			element._needAncestorCacheClear = true;
			break;

		case "width" :
		case "height" :
			element._needSizeUpdate = true;
			element._needAncestorCacheClear = true;
			element._needRedraw = true;
			break;

		case "opacity" :
			element._needOpacityUpdate = true;
			element._needRedraw = true;
			break;

		case "className" :
			element.updateProperties();
			element._needRedraw = true;
			break;

		default :
			element._needRedraw = true;
			break
	};

	element._needRefresh = true;
	element.__unlock("DOM");
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

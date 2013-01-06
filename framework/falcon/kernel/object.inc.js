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

	/* Public Properties (visual impact on element, need redraw) */
	/* Common to all elements */
	DOMElement.definePublicProperties(this, {
		// -- class management
		className : OptionalString(o.class, ""),

		// -- layout properties
		left : OptionalNumber(o.left, 0),
		top : OptionalNumber(o.top, 0),
		width : o.width ? Number(o.width) : p ? p._width : window.width,
		height : o.height ? Number(o.height) : p ? p._height : window.height,

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

		_absx : 0,
		_absy : 0,

		_layerPadding : 10,
		_cachedBackgroundImage : null,

		_needRefresh : true,
		_needRedraw : true,
		_needPositionUpdate : true,
		_needSizeUpdate : true,
		_needOpacityUpdate : true
	});

	/* Runtime changes does not impact the visual aspect of the element */
	this.id = OptionalString(o.id, this._uid);
	this.name = OptionalString(o.name, "");
	this.isOnTop = false;
	this.mouseOverPath = false;

	Native.elements.init(this);

	if (this.className != '') {
		this.updateProperties();
	}
};

/* -------------------------------------------------------------------------- */

DOMElement.prototype = {
	__noSuchMethod__ : Native.object.__noSuchMethod__,

	__lock : Native.object.__lock, // disable setter events
	__unlock : Native.object.__unlock, // enable setter events

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

	/* user customisable methods */
	update : function(context){},
	draw : function(context){}
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

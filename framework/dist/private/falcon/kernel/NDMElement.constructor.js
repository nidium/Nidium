/* ------------------------+------------- */
/* Native Framework 2.0    | Falcon Build */
/* ------------------------+------------- */
/* (c) 2013 nidium.com - Vincent Fontaine */
/* -------------------------------------- */

"use strict";

/* -------------------------------------------------------------------------- */

var NDMElement = function(type, options, parent){
	var o = this.options = options || {},
		p = this.parent = parent ? parent : null; // parent element

	if (!Native.elements[type]) {
		throw("Undefined element " + type);
	}

	if (o && o.class) {
		o.className = o.class;
	}

	this.parent = p;
	this.nodes = []; // children elements
	
	/* Read Only Properties */
	NDMElement.defineReadOnlyProperties(this, {
		type : type,
		isNDMElement : true,
		inline : {}  
	});

	for (var k in this.options) {
		if (this.options.hasOwnProperty(k)) {
			this.inline[k] = this.options[k];
		}
	}

	/*
	this.left = OptionalNumber(o.left, 0);
	this.top = OptionalNumber(o.top, 0);

	this.width = o.width ? Number(o.width) : p ?
					p._width-OptionalNumber(o.left, 0) :
					window.width-OptionalNumber(o.left, 0);

	this.height = o.height ? Number(o.height) : p ?
					p._height-OptionalNumber(o.top, 0) :
					window.height-OptionalNumber(o.top, 0);
	*/

	/* Public Dynamic Properties (visual impact on element, need redraw) */
	/* Common to all elements */
	NDMElement.defineDynamicProperties(this, {
		// -- class management
		id : null,
		className : OptionalString(o.class, ""),

		// -- layout properties
		left : OptionalNumber(o.left, 0),
		top : OptionalNumber(o.top, 0),

		width : o.width ? Number(o.width) : p ?
					p._width-OptionalNumber(o.left, 0) :
					window.width-OptionalNumber(o.left, 0),

		height : o.height ? Number(o.height) : p ?
					p._height-OptionalNumber(o.top, 0) :
					window.height-OptionalNumber(o.top, 0),

		innerWidth : o.innerWidth ? Number(o.innerWidth) : -1,
		innerHeight : o.innerHeight ? Number(o.innerHeight) : -1,

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
		paddingTop : OptionalNumber(o.paddingTop, 0),
		paddingBottom : OptionalNumber(o.paddingBottom, 0),

		// -- text related properties
		text : OptionalString(o.text, ""),
		label : OptionalString(o.label, ""),
		fontSize : OptionalNumber(o.fontSize, 12),
		fontFamily : OptionalString(o.fontFamily, "arial"),
		textAlign : OptionalAlign(o.textAlign, "left"),
		lineHeight : OptionalNumber(o.lineHeight, 18),
		fontWeight : OptionalWeight(o.fontWeight, "normal"),

		// -- icon related properties
		shape : OptionalString(o.shape, ""),
		variation : OptionalNumber(o.variation, 0),

		// -- style properties
		blur : OptionalNumber(o.blur, 0),
		opacity : OptionalNumber(o.opacity, 1),
		alpha : OptionalNumber(o.alpha, 1),

		shadowBlur : OptionalNumber(o.shadowBlur, 0),
		shadowColor : OptionalValue(o.shadowColor, "rgba(0, 0, 0, 0.5)"),
		shadowOffsetX : OptionalNumber(o.shadowOffsetX, 0),
		shadowOffsetY : OptionalNumber(o.shadowOffsetY, 0),

		textShadowBlur : OptionalNumber(o.textShadowBlur, 0),
		textShadowColor : OptionalValue(o.textShadowColor, ''),
		textShadowOffsetX : OptionalNumber(o.textShadowOffsetX, 0),
		textShadowOffsetY : OptionalNumber(o.textShadowOffsetY, 0),

		color : OptionalValue(o.color, ''),
		background : OptionalValue(o.background, ''),
		backgroundImage : OptionalValue(o.backgroundImage, ''),
		backgroundRepeat : OptionalBoolean(o.backgroundRepeat, true),
		radius : OptionalNumber(o.radius, 0, 0),
		borderWidth : OptionalNumber(o.borderWidth, 0),
		borderColor : OptionalValue(o.borderColor, ''),
		outlineColor : OptionalValue(o.outlineColor, 'blue'),

		angle : OptionalNumber(o.angle, 0),
		scale : OptionalNumber(o.scale, 1),

		// -- misc flags
		canReceiveFocus : OptionalBoolean(o.canReceiveFocus, false),
		outlineOnFocus : OptionalBoolean(o.outlineOnFocus, true),
		canReceiveKeyboardEvents : OptionalBoolean(
			o.canReceiveKeyboardEvents,
			false
		),

		visible : OptionalBoolean(o.visible, true),
		hidden : OptionalBoolean(o.hidden, false),
		selected : OptionalBoolean(o.selected, false),
		overflow : OptionalBoolean(o.overflow, true),
		multiline : OptionalBoolean(o.multiline, false),
		editable : OptionalBoolean(o.editable, false),
		disabled : OptionalBoolean(o.disabled, false),
		outline : OptionalBoolean(o.outline, false),
		
		scrollable : OptionalBoolean(o.scrollable, false),
		scrollBarX : OptionalBoolean(o.scrollBarX, false),
		scrollBarY : OptionalBoolean(o.scrollBarY, false),
		position : OptionalPosition(o.position, "relative"),

		hover : false,
		hasFocus : false,

		cursor : OptionalCursor(o.cursor, "arrow")
	});

	/* Internal Hidden Properties */
	NDMElement.defineInternalProperties(this, {
		private : {},

		flags : 0,
		lineIndex : 0,

		_root : p ? p._root : null,
		_nid : document.layout.objID++,
		_uid : "_obj_" + document.layout.objID,
		_eventQueues : [],
		_mutex : [],
		_locked : true,

		_cachedBackgroundImage : null,

		_minx : this._left,
		_miny : this._top,
		_maxx : this._left + this._width,
		_maxy : this._top + this._top,

		_layerPadding : p ? Math.max(this.shadowBlur*5, 20) : Math.max(this.shadowBlur*5, 10),

		/* refreshing flags */
		_needRefresh : true,
		_needRedraw : true,
		_needPositionUpdate : true,
		_needSizeUpdate : true,
		_needOpacityUpdate : true,
		_needAncestorCacheClear : false
	});

	Object.createProtectedHiddenElement(this, "static", {
		default : {}
	});

	/* runtime change of id leads to nss class check */
	this.id = OptionalString(o.id, this._uid);

	/* Runtime changes does not impact the visual aspect of the element */
	this.name = OptionalString(o.name, "");
	this.isOnTop = false;
	this.hasChildren = false;
	this.mouseOverPath = false;

	if (p){
		p.hasChildren = true;
	}
};

/* -------------------------------------------------------------------------- */


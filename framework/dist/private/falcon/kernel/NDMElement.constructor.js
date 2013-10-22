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

		// Hack to avoid crash when canvas.width = NaN;
		width : o.width ? Number(o.width) : p ?
					p._width-OptionalNumber(o.left, 0) :
					window.width-OptionalNumber(o.left, 0),

		// Hack to avoid crash when canvas.height = NaN;
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
		verticalAlign : "middle",
		textOffsetX : OptionalNumber(o.textOffsetX, 0),
		textOffsetY : OptionalNumber(o.textOffsetY, 0),
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


	/*
		this.id = null;
		this.className = OptionalString(o.class, "");

		// -- layout properties
		this.left = OptionalNumber(o.left, 0);
		this.top = OptionalNumber(o.top, 0);

		// Hack to avoid crash when canvas.width = NaN;
		this.width = o.width ? Number(o.width) : p ?
					p._width-OptionalNumber(o.left, 0) :
					window.width-OptionalNumber(o.left, 0);

		// Hack to avoid crash when canvas.height = NaN;
		this.height = o.height ? Number(o.height) : p ?
					p._height-OptionalNumber(o.top, 0) :
					window.height-OptionalNumber(o.top, 0);

		this.innerWidth = o.innerWidth ? Number(o.innerWidth) : -1;
		this.innerHeight = o.innerHeight ? Number(o.innerHeight) : -1;

		this.scrollLeft = OptionalNumber(o.scrollLeft, 0);
		this.scrollTop = OptionalNumber(o.scrollTop, 0);

		this.percentLeft = OptionalNumber(o.percentLeft, 0);
		this.percentTop = OptionalNumber(o.percentTop, 0);
		this.percentWidth = o.percentWidth ? Number(o.percentWidth) : 100;
		this.percentHeight = o.percentHeight ? Number(o.percentHeight) : 100;

		this.offsetLeft = OptionalNumber(o.offsetLeft, 0);
		this.offsetTop = OptionalNumber(o.offsetTop, 0);

		this.paddingLeft = OptionalNumber(o.paddingLeft, 0);
		this.paddingRight = OptionalNumber(o.paddingLeft, 0);
		this.paddingTop = OptionalNumber(o.paddingTop, 0);
		this.paddingBottom = OptionalNumber(o.paddingBottom, 0);

		// -- text related properties
		this.text = OptionalString(o.text, "");
		this.label = OptionalString(o.label, "");
		this.fontSize = OptionalNumber(o.fontSize, 12);
		this.fontFamily = OptionalString(o.fontFamily, "arial");
		this.textAlign = OptionalAlign(o.textAlign, "left");
		this.verticalAlign = "middle";
		this.textOffsetX = OptionalNumber(o.textOffsetX, 0);
		this.textOffsetY = OptionalNumber(o.textOffsetY, 0);
		this.lineHeight = OptionalNumber(o.lineHeight, 18);
		this.fontWeight = OptionalWeight(o.fontWeight, "normal");

		// -- icon related properties
		this.shape = OptionalString(o.shape, "");
		this.variation = OptionalNumber(o.variation, 0);

		// -- style properties
		this.blur = OptionalNumber(o.blur, 0);
		this.opacity = OptionalNumber(o.opacity, 1);
		this.alpha = OptionalNumber(o.alpha, 1);

		this.shadowBlur = OptionalNumber(o.shadowBlur, 0);
		this.shadowColor = OptionalValue(o.shadowColor, "rgba(0, 0, 0, 0.5)");
		this.shadowOffsetX = OptionalNumber(o.shadowOffsetX, 0);
		this.shadowOffsetY = OptionalNumber(o.shadowOffsetY, 0);

		this.textShadowBlur = OptionalNumber(o.textShadowBlur, 0);
		this.textShadowColor = OptionalValue(o.textShadowColor, '');
		this.textShadowOffsetX = OptionalNumber(o.textShadowOffsetX, 0);
		this.textShadowOffsetY = OptionalNumber(o.textShadowOffsetY, 0);

		this.color = OptionalValue(o.color, '');
		this.background = OptionalValue(o.background, '');
		this.backgroundImage = OptionalValue(o.backgroundImage, '');
		this.backgroundRepeat = OptionalBoolean(o.backgroundRepeat, true);
		this.radius = OptionalNumber(o.radius, 0, 0);
		this.borderWidth = OptionalNumber(o.borderWidth, 0);
		this.borderColor = OptionalValue(o.borderColor, '');
		this.outlineColor = OptionalValue(o.outlineColor, 'blue');

		this.angle = OptionalNumber(o.angle, 0);
		this.scale = OptionalNumber(o.scale, 1);

		// -- misc flags
		this.canReceiveFocus = OptionalBoolean(o.canReceiveFocus, false);
		this.outlineOnFocus = OptionalBoolean(o.outlineOnFocus, true);
		this.canReceiveKeyboardEvents = OptionalBoolean(
			o.canReceiveKeyboardEvents,
			false
		),

		this.visible = OptionalBoolean(o.visible, true);
		this.hidden = OptionalBoolean(o.hidden, false);
		this.selected = OptionalBoolean(o.selected, false);
		this.overflow = OptionalBoolean(o.overflow, true);
		this.multiline = OptionalBoolean(o.multiline, false);
		this.editable = OptionalBoolean(o.editable, false);
		this.disabled = OptionalBoolean(o.disabled, false);
		this.outline = OptionalBoolean(o.outline, false);
		
		this.scrollable = OptionalBoolean(o.scrollable, false);
		this.scrollBarX = OptionalBoolean(o.scrollBarX, false);
		this.scrollBarY = OptionalBoolean(o.scrollBarY, false);
		this.position = OptionalPosition(o.position, "relative");

		this.hover = false;
		this.hasFocus = false;

		this.cursor = OptionalCursor(o.cursor, "arrow");
	*/

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


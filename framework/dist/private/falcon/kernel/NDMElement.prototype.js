/* ------------------------+------------- */
/* Native Framework 2.0    | Falcon Build */
/* ------------------------+------------- */
/* (c) 2013 nidium.com - Vincent Fontaine */
/* -------------------------------------- */

"use strict";

/* -------------------------------------------------------------------------- */

NDMElement.prototype = {
	__noSuchMethod__ : NDMElement.method.__noSuchMethod__,

	/* ----------------------------------------------- */

	__lock : NDMElement.method.__lock, // disable setter events
	__unlock : NDMElement.method.__unlock, // enable setter events
	__refresh : NDMElement.method.__refresh, // internal refresh
	__updateLayerOpacity : NDMElement.method.__updateLayerOpacity,
	__updateLayerPosition : NDMElement.method.__updateLayerPosition,
	__updateLayerSize : NDMElement.method.__updateLayerSize,
	__updateAncestors : NDMElement.method.__updateAncestors,
	__resizeLayer : NDMElement.method.__resizeLayer,

	/* ----------------------------------------------- */

	add : NDMElement.method.add,
	remove : NDMElement.method.remove,

	clone : NDMElement.method.clone,
	cloneNode : NDMElement.method.cloneNode,

	clear : NDMElement.method.clear,
	show : NDMElement.method.show,
	hide : NDMElement.method.hide,
	focus : NDMElement.method.focus,

	addChild : NDMElement.method.addChild,
	appendChild : NDMElement.method.addChild,
	removeChild : NDMElement.method.removeChild,
	insertBefore : NDMElement.method.insertBefore,
	insertAfter : NDMElement.method.insertAfter,
	insertChildAtIndex : NDMElement.method.insertChildAtIndex,

	getChildren : NDMElement.method.getChildren,

	beforeDraw : NDMElement.method.beforeDraw,
	afterDraw : NDMElement.method.afterDraw,

	isPointInside : NDMElement.method.isPointInside,
	isVisible : NDMElement.method.isVisible,
	isAncestor : NDMElement.method.isAncestor,
	isBoundBySelector : NDMElement.method.isBoundBySelector,

	getDrawingBounds : NDMElement.method.getDrawingBounds,
	getBoundingRect : NDMElement.method.getBoundingRect,

	hasClass : NDMElement.method.hasClass,
	addClass : NDMElement.method.addClass,
	removeClass : NDMElement.method.removeClass,

	getPropertyHandler : NDMElement.method.getPropertyHandler,
	applySelectorProperties : NDMElement.method.applySelectorProperties,
	applyStyleSheet : NDMElement.method.applyStyleSheet,
	applyInlineProperties : NDMElement.method.applyInlineProperties,
	copyInlinePropertiesFrom : NDMElement.method.copyInlinePropertiesFrom,

	bringToFront : NDMElement.method.bringToFront,
	sendToBack : NDMElement.method.sendToBack,
	resetNodes : NDMElement.method.resetNodes,

	setProperties : NDMElement.method.setProperties,

	center : NDMElement.method.center,
	centerLeft : NDMElement.method.centerLeft,
	centerTop : NDMElement.method.centerTop,
	move : NDMElement.method.move,
	place : NDMElement.method.place,
	setCoordinates : NDMElement.method.setCoordinates,
	fix : NDMElement.method.fix,

	redraw : NDMElement.method.redraw,

	expand : NDMElement.method.expand,
	shrink : NDMElement.method.shrink,

	/* ----------------------------------------------- */
	/* Plugin Customisable Methods                     */
	/* ----------------------------------------------- */

	onAdoption : function(parent){},
	onAddChildRequest : function(child){},
	onChildReady : function(child){},

	resize : function(){},
	update : function(e){},
	draw : function(context){},


	/* ----------------------------------------------- */
	/* NDMElement.maxWidth                             */
	/* ----------------------------------------------- */

	set maxWidth(value) {
		var w = value == null || value == '' ? null : Number(value);

		if (w === null) {
			w = this.contentWidth;
			this.width = w;
		} else {
			this.width = Math.min(this._width, w);
		}

		this._maxWidth = w;

		/* update element's inline property */
		!this._locked && (this.inline.maxWidth = w);
	},

	get maxWidth() {
		return this._maxWidth;
	},

	/* ----------------------------------------------- */
	/* NDMElement.fastLeft                             */
	/* ----------------------------------------------- */

	set fastLeft(value) {
		this._left = value;
		!this._locked && (this.inline.left = value);
		this.layer.left = value;
	},

	/* ----------------------------------------------- */
	/* NDMElement.fastTop                              */
	/* ----------------------------------------------- */

	set fastTop(value) {
		this._top = value;
		!this._locked && (this.inline.top = value);
		this.layer.top = value;
	},

	/* ----------------------------------------------- */
	/* NDMElement.left                                 */
	/* ----------------------------------------------- */
	/*
	set left(value) {
		this._left = value;
		!this._locked && (this.inline.left = value);
		if (this.layer) {
			this.layer.left = value;
			this.__updateAncestors();
		}
	},

	get left() {
		return this._left;
	},
	*/
	/* ----------------------------------------------- */
	/* NDMElement.top                                  */
	/* ----------------------------------------------- */
	/*
	set top(value) {
		this._top = value;
		!this._locked && (this.inline.top = value);
		if (this.layer) {
			this.layer.top = value;
			this.__updateAncestors();
		}
	},

	get top() {
		return this._top;
	},
	*/
	/* ----------------------------------------------- */
	/* NDMElement.width                                */
	/* ----------------------------------------------- */
	/*
	set width(value) {
		this._width = value;
		if (this.layer) {
			//this.layer.width = Math.round(value);
			this.__updateAncestors();
			this.redraw();
		}
	},

	get width() {
		return this._width;
	},
	*/

	/* ----------------------------------------------- */
	/* NDMElement.height                               */
	/* ----------------------------------------------- */
	/*
	set height(value) {
		this._height = value;
		if (this.layer) {
			//this.layer.height = Math.round(value);
			this.__updateAncestors();
			this.redraw();
		}
	},

	get height() {
		return this._top;
	},
	*/


	/* ----------------------------------------------- */
	/* READ ONLY WRAPPERS                              */
	/* ----------------------------------------------- */

	get __left() {
		return this.layer ? this.layer.__left : 0;
	},

	get __top() {
		return this.layer ? this.layer.__top : 0;
	},

	get contentWidth() {
		return this.layer ? this.layer.contentWidth : this.width;
	},

	get contentHeight() {
		return this.layer ? this.layer.contentHeight : this.height;
	},

	get children() {
		return this.getChildren();
	},

	get previousSibling() {
		var layer = this.layer ? this.layer.getPrevSibling() : null;
		return layer ? layer.host : null;
	},

	get nextSibling() {
		var layer = this.layer ? this.layer.getNextSibling() : null;
		return layer ? layer.host : null;
	},

	get childNodes() {
		return this.nodes;
	},

	get parentNode() {
		return this.parent ? this.parent : null;
	},

	get nodeType() {
		return this.type;
	},

	get nodeName() {
		return this.name;
	},

	get nodeIndex() {
		return this.parent ? this.parent.nodes.indexOf(this) : -1;
	},

	get ownerDocument() {
		return this._root;
	},

	get hasChildNodes() {
		return this.nodes.length>0;
	},

	get hasOwnerDocument() {
		return this._root !== null;
	},

	get orphaned() {
		return this.parent === null && this._root === null;
	},

	get rooted() {
		return !this.orphaned;
	},

	/* ----------------------------------------------- */
	/* EVENTS                                          */
	/* ----------------------------------------------- */

	events : {
		onbeforecopy : null, /* implemented */
		onbeforecut : null, /* implemented */
		onbeforepaste : null, /* implemented */
		oncopy : null, /* implemented */
		oncut : null, /* implemented */
		onpaste : null, /* implemented */
		onselect : null, /* implemented */
		onselectstart : null, /* implemented */

		onblur : null, /* implemented */
		onfocus : null, /* implemented */

		onchange : null, /* implemented */
		oncontextmenu : null, /* implemented */

		ondrag : null, /* implemented */
		ondragend : null, /* implemented */
		ondragenter : null, /* implemented */
		ondragleave : null, /* implemented */
		ondragover : null, /* implemented */
		ondragstart : null, /* implemented */
		ondrop : null, /* implemented */

		onkeydown : null, /* implemented */
		onkeypress : null, /* implemented */
		onkeyup : null, /* implemented */
		ontextinput : null, /* implemented */
		onload : null, /* implemented */

		onmouseclick : null, /* implemented */
		onmousedblclick : null, /* implemented */
		onmousedown : null, /* implemented */
		onmousemove : null, /* implemented */
		onmouseout : null, /* implemented */
		onmouseover : null, /* implemented */
		onmouseup : null, /* implemented */
		onmousewheel : null, /* implemented */

		onreset : null, /* implemented */
		onerror : null, /* implemented */
		onsubmit : null, /* implemented */

		onscroll : null
	},

	/* ----------------------------------------------- */
	/* NSS Properties (to be implemented)              */
	/* ----------------------------------------------- */

	/*
	backgroundColor : "",
	backgroundImage : "",      ** implemented **
	backgroundPositionX : "",
	backgroundPositionY : "",
	backgroundRepeat : "",

	border : "",      ** implemented **
	borderBottom : "",
	borderBottomColor : "",
	borderBottomLeftRadius : "",
	borderBottomRightRadius : "",
	borderBottomStyle : "",
	borderBottomWidth : "",
	borderCollapse : "",
	borderColor : "",      ** implemented **
	borderImage : "",
	borderImageOutset : "",
	borderImageRepeat : "",
	borderImageSlice : "",
	borderImageSource : "",
	borderImageWidth : "",
	borderLeft : "",
	borderLeftColor : "",
	borderLeftStyle : "",
	borderLeftWidth : "",
	borderRadius : "",
	borderRight : "",
	borderRightColor : "",
	borderRightStyle : "",
	borderRightWidth : "",
	borderSpacing : "",
	borderStyle : "",
	borderTop : "",
	borderTopColor : "",
	borderTopLeftRadius : "",
	borderTopRightRadius : "",
	borderTopStyle : "",
	borderTopWidth : "",
	borderWidth : "",       ** implemented **

	clipPath : "",
	clipRule : "",

	colorInterpolation : "",
	colorInterpolationFilters : "",
	colorRendering : "",

	direction : "",
	display : "",

	fill : "",
	fillOpacity : "",
	fillRule : "",

	filter : "",
	float : "",

	floodColor : "",
	floodOpacity : "",

	font : "",
	fontFamily : "",
	fontSize : "",      ** implemented **
	fontStretch : "",
	fontStyle : "",
	fontVariant : "",
	fontWeight : "",

	imageSmoothingEnabled : "",

	kerning : "",
	letterSpacing : "",
	lightingColor : "",
	lineHeight : "",      ** implemented **

	listStyle : "",
	listStyleImage : "",
	listStylePosition : "",
	listStyleType : "",

	margin : "",
	marginBottom : "",
	marginLeft : "",
	marginRight : "",
	marginTop : "",

	maxHeight : "",
	maxWidth : "",
	minHeight : "",
	minWidth : "",

	outline : "",      ** implemented **
	outlineColor : "",      ** implemented **
	outlineOffset : "",
	outlineStyle : "",
	outlineWidth : "",

	overflow : "",
	overflowWrap : "",
	overflowX : "",
	overflowY : "",

	padding : "",
	paddingBottom : "",
	paddingLeft : "",
	paddingRight : "",
	paddingTop : "",

	strokeDasharray : "",
	strokeDashoffset : "",
	strokeLinecap : "",
	strokeLinejoin : "",
	strokeMiterlimit : "",
	strokeOpacity : "",
	strokeWidth : "",

	tabSize : "",

	textAlign : "",
	textAnchor : "",
	textDecoration : "",
	textIndent : "",
	textLineThrough : "",
	textLineThroughColor : "",
	textLineThroughMode : "",
	textLineThroughStyle : "",
	textLineThroughWidth : "",
	textOverflow : "",
	textOverline : "",
	textOverlineColor : "",
	textOverlineMode : "",
	textOverlineStyle : "",
	textOverlineWidth : "",
	textRendering : "",
	textShadow : "",      ** implemented **
	textTransform : "",
	textUnderline : "",
	textUnderlineColor : "",
	textUnderlineMode : "",
	textUnderlineStyle : "",
	textUnderlineWidth : "",

	verticalAlign : "",
	visibility : "",
	whiteSpace : "",
	zIndex : "",
	zoom : ""
	*/
};

/* -------------------------------------------------------------------------- */

NDMElement.implement = function(props){
	Object.merge(NDMElement.method, props);
	for (var key in props){
		if (props.hasOwnProperty(key)){
			NDMElement.prototype[key] = NDMElement.method[key];
		}
	}
};

var	isNDMElement = function(element){
	return element && element.isNDMElement;
};

/* -------------------------------------------------------------------------- */

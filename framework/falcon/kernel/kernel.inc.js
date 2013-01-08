/* ------------------------+------------- */
/* Native Framework 2.0    | Falcon Build */
/* ------------------------+------------- */
/* (c) 2013 Stight.com - Vincent Fontaine */
/* -------------------------------------- */

"use strict";

/* -------------------------------------------------------------------------- */
var hhh =0;
Native.object = {
	__noSuchMethod__ : function(id, args){
		throw("Undefined method " + id);
	},

	__lock : function __lock(){
		this._locked = true;
	},

	__unlock : function __unlock(){
		this._locked = false;
	},

	refresh : function refresh(){
		var p = this.parent,
			x = this._left + this._offsetLeft,
			y = this._top + this._offsetTop;

		this._absx = p ? p._absx + x : x;
		this._absy = p ? p._absy + y : y;

		this.__layerPadding = p ? p._layerPadding - this._layerPadding 
								: this._layerPadding;

		this.layer.visible = this._visible;
		
		if (this._needOpacityUpdate){
			this.layer.context.globalAlpha = this._opacity;
			this._needOpacityUpdate = false;
		}

		if (this._needPositionUpdate){
			this.layer.left = this._left + this.__layerPadding;
			this.layer.top = this._top + this.__layerPadding;
			this._needPositionUpdate = false;
			this._needContentSizeUpdate = true;
		}

		if (this._needSizeUpdate){
			var w = this.getLayerPixelWidth(),
				h = this.getLayerPixelHeight();
			this.layer.width = w;
			this.layer.height = h;
			this._needSizeUpdate = false;
			this._needRedraw = true;
			this._needContentSizeUpdate = true;
		}

		if (this._needContentSizeUpdate){
			if (Native.__debugger && this._root!=Native.__debugger){
				Native.layout.updateInnerContentTree(this);
				this._needContentSizeUpdate = false;
			}
		}

		if (this._needRedraw) {
			this.layer.clear();
			if (this.layer.debug) this.layer.debug();
			this.draw(this.layer.context);
			this._needRedraw = false;
		}

		this._needRefresh = false;
	},

	add : function add(type, options){
		var element = new DOMElement(type, options, this);
		this.addChild(element);
		return element;
	},

	remove : function remove(){
		Native.layout.remove(this);
		Native.layout.update();
	},

	show : function show(){
		if (!this.visible) {
			this.visible = true;
		}
		return this;
	},

	hide : function hide(){
		if (this.visible) {
			this.visible = false;
		}
		return this;
	},

	focus : function focus(){
		return this;
	},

	addChild : function addChild(element){
		if (this.nodes[element._uid] || !isDOMElement(element)) return false;
		this.nodes[element._uid] = element;
		element._root = this._root;
		element.parent = this;
		element.parent.layer.add(element.layer);
		Native.layout.update();
	},

	removeChild : function removeChild(element){
		if (element.parent && element.parent != this){
			throw("Unable to remove this element.");
		}
		Native.layout.remove(element);
		Native.layout.update();
	},

	getLayerPixelWidth : function getLayerPixelWidth(){
		return Math.round(this._width + 2*this._layerPadding);
	},

	getLayerPixelHeight : function getLayerPixelHeight(){
		return Math.round(this._height + 2*this._layerPadding);
	},

	/*
	 * Sort DOMElements to match hardware physical layers order.
	 */
	resetNodes : function resetNodes(){
		if (!this.parent) return false;

		var parent = this.parent, // parent of this virtual element
			layers = parent.layer.getChildren(); // physical children

		/* Reset parent's nodes */
		parent.nodes = {};

		/* reconstruct the nodes in the right order */
		for (var i in layers){
			// get the host element (that's it : the virtual element)
			var element = layers[i].host;

			// add the element in parent's nodes 
			parent.nodes[element._uid] = element;
		}

		Native.layout.update();
	},

	bringToFront : function bringToFront(){
		this.layer.bringToFront();
		this.resetNodes();
		return this;
	},

	sendToBack : function sendToBack(){
		this.layer.sendToBack();
		this.resetNodes();
		return this;
	},

	getDrawingBounds : function getDrawingBounds(){
		var p = this.parent;
		return {
			x : 0 + this.offsetLeft + this._layerPadding,
			y : 0 + this.offsetTop + this._layerPadding,
			w : this.width,
			h : this.height,
			textOffsetX : 0,
			textOffsetY : 0
		};
	},

	beforeDraw : function beforeDraw(){

	},

	afterDraw : function afterDraw(){

	},

	isPointInside : function isPointInside(mx, my){
		this._absx = Math.round(this.layer.__left + this._layerPadding);
		this._absy = Math.round(this.layer.__top + this._layerPadding);

		var x1 = this._absx+1,
			y1 = this._absy+2,
			x2 = x1 + this._width,
			y2 = y1 + this._height;

		return (mx>=x1 && mx<x2 && my>=y1 && my<y2) ? true : false;
	},

	isVisible : function isVisible(){
		return this.visible;
	},

	hasClass : function hasClass(name){
		return new RegExp('(\\s|^)'+name+'(\\s|$)').test(this.className);
	},

	addClass : function addClass(name){
		if (!this.hasClass(name)){
			this.className += (this.className ? ' ' : '') + name;
		}
		return this;
	},

	setClass : function setClass(name){
		this.className = name;
		return this;
	},

	removeClass : function removeClass(name){
		if (this.hasClass(name)){
			let r = new RegExp('(\\s|^)'+name+'(\\s|$)'),
				k = this.className;

			this.className = k.replace(r,' ').replace(/^\s+|\s+$/g, '');
		}
		return this;
	},

	updateProperties : function(){
		var classNames = this.className.split(" ");

		for (var i in classNames){
			var props = Native.StyleSheet.getProperties(classNames[i]);

			this.setProperties(props);
		}
	},

	setProperties : function setProperties(options){
		for (var key in options){
			if (options.hasOwnProperty(key)){
				this[key] = options[key];
			}
		}
		return this;
	},

	toString : function toString(){
		return "Object Manager";
	}
};

/* -------------------------------------------------------------------------- */

Native.StyleSheet = {
	document : {},

	/* add to the existing stylesheet document */
	add : function(sheet){
		for (var k in sheet){
			if (sheet.hasOwnProperty(k)){
				if (this.document[k]){
					this.mergeProperties(k, sheet[k]);
				} else {
					this.document[k] = sheet[k];
				}

				Native.layout.getElementsByClassName(k).each(function(){
					this.updateProperties();
				});
			}
		}
	},

	/* replace the existing stylesheet document */
	set : function(sheet){
		this.document = {};
		this.add(sheet);
	},

	/* load a local or distant stylesheet (asynchronous) */
	load : function(url){
		var self = this;

		File.getText(url, function(content){
			var sheetText = self.parse(content);
			try {
				eval("self.add(" + sheetText + ")");
			} catch (e) {
				throw ('Error parsing Native StyleSheet "'+url+'"');
			}
		});
	},

	mergeProperties : function(klass, properties){
		var prop = this.document[klass];
		for (var p in properties){
			if (properties.hasOwnProperty(p)){
				prop[p] = properties[p];
			}
		}
	},

	getProperties : function(klass){
		return this.document[klass];
	},

	/*
	 * adapted from James Padolsey's work
	 */
	parse : function(text){
		text = ('__' + text + '__').split('');

		var mode = {
			singleQuote : false,
			doubleQuote : false,
			regex : false,
			blockComment : false,
			lineComment : false
		};

		for (var i = 0, l = text.length; i < l; i++){
			if (mode.regex){
				if (text[i] === '/' && text[i-1] !== '\\'){
					mode.regex = false;
				}
				continue;
			}

			if (mode.singleQuote){
				if (text[i] === "'" && text[i-1] !== '\\'){
					mode.singleQuote = false;
				}
				continue;
			}

			if (mode.doubleQuote){
				if (text[i] === '"' && text[i-1] !== '\\'){
					mode.doubleQuote = false;
				}
				continue;
			}

			if (mode.blockComment){
				if (text[i] === '*' && text[i+1] === '/'){
				text[i+1] = '';
					mode.blockComment = false;
				}
				text[i] = '';
				continue;
			}

			if (mode.lineComment){
				if (text[i+1] === '\n' || text[i+1] === '\r'){
					mode.lineComment = false;
				}
				text[i] = '';
				continue;
			}

			mode.doubleQuote = text[i] === '"';
			mode.singleQuote = text[i] === "'";

			if (text[i] === '/'){
				if (text[i+1] === '*'){
					text[i] = '';
					mode.blockComment = true;
					continue;
				}
				if (text[i+1] === '/'){
					text[i] = '';
					mode.lineComment = true;
					continue;
				}
				mode.regex = true;
			}
		}

		return text.join('')
				.slice(2, -2)
				.replace(/[\n\r]/g, '')
				.replace(/\s+/g, ' ');
	}
};

/* -------------------------------------------------------------------------- */


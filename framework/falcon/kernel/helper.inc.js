/* ------------------------+------------- */
/* Native Framework 2.0    | Falcon Build */
/* ------------------------+------------- */
/* (c) 2013 Stight.com - Vincent Fontaine */
/* -------------------------------------- */

"use strict";

/* -------------------------------------------------------------------------- */

Object.merge = function(obj, props){
	obj = obj || {};
	for (var key in props){
		if (props.hasOwnProperty(key)){
			obj[key] = props[key];
		}
	}
};

Object.createProtectedElement = function(scope, property, value){
	Object.defineProperty(scope, property, {
		value : value,
		enumerable : true,
		writable : false,
		configurable : false
	});
};

Object.createHiddenElement = function(scope, property, value){
	Object.defineProperty(scope, property, {
		value : value,
		enumerable : false,
		writable : true,
		configurable : false
	});
};

Object.createProtectedHiddenElement = function(scope, property, value){
	Object.defineProperty(scope, property, {
		value : value,
		enumerable : false,
		writable : false,
		configurable : false
	});
};

/* -------------------------------------------------------------------------- */

Object.merge(window, {
	width : Native.canvas.width,
	height : Native.canvas.height,
	mouseX : 0,
	mouseY : 0,
	requestAnimationFrame : Native.canvas.getContext("2D").requestAnimationFrame,

	navigator : {
		get appName() {
			return "NATiVE";
		},

		get appVersion() {
			return "1.0";
		},

		get platform() {
			return "MacOSX";
		},

		get userAgent() {
			return "Stight/1.0 (en-US; rv:1.0.3) Falcon "+ಠ_ಠ;
		}
	}
});

/* -------------------------------------------------------------------------- */

// Syntax : "22".in([1, 23], 21, {"22":true}) equal true

Object.in = function(...n){
	var	match = false;
	for (var i in n){
		if (n[i].constructor == String){
			if (this == n[i]) {
				match = true;
				break;
			}
		} else {
			for (var j in n[i]){
				if (this == n[i][j] || this == j) {
					match = true;
					break;
				}
			}
		}

	}
	return match;
};

String.prototype.in = Object.in;
Number.prototype.in = Object.in;

/* -------------------------------------------------------------------------- */

String.prototype.leftPad = function(len, sep){
	return Array(len + 1 - this.length).join(sep?sep:" ") + this;
};

String.prototype.rightPad = function(len, sep){
	return this + Array(len + 1 - this.length).join(sep?sep:" ");
}

String.prototype.clean = function(){
	return this.replace(/([^\/"']+|"(?:[^"\\]|\\.)*"|'(?:[^'\\]|\\.)*')|\/\*(?:[^*]|\*+[^*\/])*\*+\/|\/\/.*/, '');
};

/* -------------------------------------------------------------------------- */

Uint8Array.prototype.toString = function(){
	return String.fromCharCode.apply(null, new Uint8Array(this));
};

ArrayBuffer.prototype.toString = function(){
	return String.fromCharCode.apply(null, new Uint8Array(this));
};

String.prototype.toUint8Array = function(){
	var size = this.length,
		buffer = new ArrayBuffer(size),
		view = new Uint8Array(buffer);

	for (var i=0; i<size; i++) {
		view[i] = this.charCodeAt(i);
	}
	return buffer;
};

String.prototype.toUint16Array = function(){
	var size = this.length,
		buffer = new ArrayBuffer(size*2),
		view = new Uint16Array(buffer);

	for (var i=0; i<size; i++) {
		view[i] = this.charCodeAt(i);
	}
	return buffer;
};

/* -------------------------------------------------------------------------- */

Canvas.prototype.clear = function(){
	var context = this.getContext("2D");
	context.clearRect(
		-this.padding,
		-this.padding, 
		this.clientWidth,
		this.clientHeight
	);
};

Canvas.prototype.debug = function(){};

Canvas.implement = function(props){
	Object.merge(CanvasRenderingContext2D.prototype, props);
};

/* -------------------------------------------------------------------------- */

var toULong = function(x){
	return x >>> 0; // Uint32
};

var toLong = function(x){
	return x & 0xFFFFFFFF; // Int32
};

var toUShort = function(x){
	return (x >>> 0) & 0xFFFF; // Uint32 >> truncate.
};

var OptionalString = function(x, def){
	return x === null || x === undefined ? String(def) : String(x);
};

var OptionalValue = function(x, def){
	return x === null || x === undefined ? def : x;
};

var OptionalNumber = function(x, def, min, max){
	var p = x === null || x === undefined ? Number(def) : Number(x);
	p = (min != undefined) ? Math.max(Number(min), p) : p;
	return (max != undefined) ? Math.min(Number(max), p) : p;
};

var OptionalBoolean = function(x, def){
	return x === null || x === undefined ? Boolean(def) : Boolean(x);
};

var OptionalCallback = function(x, def){
	return typeof x === "function" ? x : def;
};

var OptionalAlign = function(x, def){
	var list = ["left", "right", "justify", "center"];
	return x && x.in(list) ? String(x) : 
				def && def.in(list) ? String(def) : null;
};

/* -------------------------------------------------------------------------- */

Number.prototype.bound = function(min, max){
	return Math.min(Math.max(Number(min), this), Number(max));
};

/* -------------------------------------------------------------------------- */

Math.distance = function(x1, y1, x2, y2){
	var a = y2-y1, b = x2-x1;
	return Math.sqrt(a*a+b*b);
};

/* -- Precalc !n */
Math.factorial = (function(n){
	var c = [],
		f = function(n){
			if (n==0 || n==1) return 1;
			if (c[n]) return c[n];
			for (var r=i=1; i<=n; i++){r *= i}
			return c[n] = r;
		};
	for (var i=0; i<500; i++) f(i);
	return f;
}());

/* -------------------------------------------------------------------------- */

var ___filter___ = function(txt, keyword){
	return txt.toLowerCase().indexOf(keyword) != -1;
};

var print = function(txt, element){
	if (element && element._root == Native.__debugger) return false;
	if (window.keydown != 1073742051) return false;

	var __LOG_BEFORE_DOMREADY__ = false,

		__KEYWORD__ = "",
		__ELEMENT_ID__ = "bigview",

		/* -- displays ---------- */
		__events__ = false,
		__keys__ = false,
		__mousemove__ = false,

		__add__ = false,
		__init__ = false,

		__draw__ = false,
		__refresh__ = true,
		__update__ = true,

		__getter__ = true,
		__setter__ = true,
		__plugin__ = true,

		__locks__ = false,
		__DOMElement__ = true,

		__other__ = false;

	if (__LOG_BEFORE_DOMREADY__ === false) {
		if (!Native.scope.document) return false;
		if (!Native.scope.document.ready) return false;
	}

	if (!__events__ && ___filter___(txt, "event")) return false; else
	if (!__keys__ && ___filter___(txt, "key")) return false; else
	if (!__mousemove__ && ___filter___(txt, "mousemove")) return false; else
	if (!__getter__ && ___filter___(txt, "get")) return false; else
	if (!__setter__ && ___filter___(txt, "set")) return false; else
	if (!__plugin__ && ___filter___(txt, "plugin")) return false; else
	if (!__locks__ && ___filter___(txt, "lock")) return false; else
	if (!__add__ && ___filter___(txt, "add")) return false; else
	if (!__init__ && ___filter___(txt, "init")) return false; else
	if (!__draw__ && ___filter___(txt, "draw")) return false; else
	if (!__update__ && ___filter___(txt, "update")) return false; else
	if (!__refresh__ && ___filter___(txt, "refresh")) return false; else
	if (!__DOMElement__ && ___filter___(txt, "domelement")) return false;

	if (__KEYWORD__ && !___filter___(txt, __KEYWORD__)) return false;

	if (element) {

		if (__ELEMENT_ID__ && element.id != __ELEMENT_ID__) return false;

		echo(
			'#'+element._root.id + ':' + element.type,
			element._uid, 
			(element.id!=element._uid?'"#'+element.id+'"':''),
			(element._label?'('+element._label+')':''),
			(element._name ? element._name : ''),
			': ' + txt
		);
	} else {
		if (__other__) echo(txt);
	}
};


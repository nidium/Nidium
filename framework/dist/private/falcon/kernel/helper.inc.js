/* ------------------------+------------- */
/* Native Framework 2.0    | Falcon Build */
/* ------------------------+------------- */
/* (c) 2013 nidium.com - Vincent Fontaine */
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

Object.createFrozenProperty = function(scope, property, value){
	Object.defineProperty(scope, property, {
		value : value,
		enumerable : true,
		writable : true,
		configurable : false
	});
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

Object.definePrivateProperties = function(element, props){
	for (var key in props){
		if (props.hasOwnProperty(key)){
			Object.createProtectedHiddenElement(element, key, props[key]);
		}
	}
};

/* -------------------------------------------------------------------------- */

Object.merge(window, {
	mouseX : 0,
	mouseY : 0,
	cursor : "arrow",
	scope : this,
	
	navigator : {
		get appName() {
			return "nidium";
		},

		get appVersion() {
			return "1.0";
		},

		get platform() {
			return "MacOSX";
		},

		get userAgent() {
			return "nidium/0.1 (en-US; rv:1.0.3) Falcon "+ಠ_ಠ;
		}
	}
});

window.canvas.context = window.canvas.getContext("2D");

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

Array.implement = function(props){
	for (var key in props){
		if (props.hasOwnProperty(key)){
			Object.createHiddenElement(Array.prototype, key, props[key]);
		}
	}
};

Array.implement({
	scroll : function(d){
		if (!d) return this;
		return this.slice(d, this.length).concat(this.slice(0, d));
	},

	each : function(cb){
		for (var i=0; i<this.length; i++) {
			cb.call(this[i]);
		}
	},

	find : function(className){
		var pattern = new RegExp("(^|\\s)"+className+"(\\s|$)"),
			z = this,
			elements = [];

		for (var i=0; i<z.length; i++){
			pattern.test(z[i]._className) && elements.push(z[i]);
		}
		return elements;
	},

	max : function(){
		return Math.max.apply(null, this);
	},

	min : function(){
		return Math.min.apply(null, this)
	}

});

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

String.prototype.splice = function(offset, size, insert){
	return (
		this.slice(0,offset) + 
		OptionalString(insert, '') + 
		this.slice(offset + Math.abs(size))
	);
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
	return typeof x === "function" ? x : def || function(){};
};

var OptionalAlign = function(x, def){
	var list = ["left", "right", "justify", "center"];
	return x && x.in(list) ? String(x) : 
				def && def.in(list) ? String(def) : null;
};

var OptionalPosition = function(x, def){
	var list = ["relative", "absolute", "fixed"];
	return x && x.in(list) ? String(x) : 
				def && def.in(list) ? String(def) : null;
};

var OptionalWeight = function(x, def){
	var list = ["normal", "bold", "bolder"];
	return x && x.in(list) ? String(x) : 
				def && def.in(list) ? String(def) : null;
};

var OptionalCursor = function(x, def){
	var list = ["arrow", "beam", "pointer", "drag"];
	return x && x.in(list) ? String(x) : 
				def && def.in(list) ? String(def) : null;
};

/* -------------------------------------------------------------------------- */

Number.prototype.bound = function(min, max){
	return Math.min(Math.max(Number(min), this), Number(max));
};

/* -------------------------------------------------------------------------- */

Math.rotate = function(x, y, cx, cy, angle){
	var cos = Math.cos(angle),
		sin = Math.sin(angle),
		ax = x - cx,
		ay = y - cy;
	return {
		x : cx + ax*cos + ay*sin,
		y : cy - ax*sin + ay*cos
	};
};

Math.distance = function(x1, y1, x2, y2){
	var a = y2-y1, b = x2-x1;
	return Math.sqrt(a*a + b*b);
};

Math.spline = function(i, n, t){
	var f = Math.factorial, pw = Math.pow;
	return f(n) / (f(i) * f(n-i)) * pw(t, i) * pw(1-t, n-i);
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

var natlog = function(txt){
	if (window.scope.NatBug && window.scope.NatBug.console) {
		NatBug.console.log(txt);
	} else {
		console.log(txt);
	}
};

/* -------------------------------------------------------------------------- */

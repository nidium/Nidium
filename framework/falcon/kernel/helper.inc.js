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
			return "Stight/1.0 (en-US; rv:1.0.2) Falcon "+ಠ_ಠ;
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

String.prototype.leftPad = function(len){
	return Array(len + 1 - this.length).join(".") + this;
};

String.prototype.rightPad = function(len){
	return this + Array(len + 1 - this.length).join(".");
}

/* -------------------------------------------------------------------------- */

Canvas.prototype.clear = function(){
	var context = this.getContext("2D");
	context.clearRect(
		0, 
		0, 
		this.width,
		this.height
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

var console = {
	iteration : 0,
	maxIterations : 20,

	log : function(...n){
		for (var i in n){
			echo.call(this, n[i]);
		}
	},

	dump : function(object){
		var self = this,
			visited = [],
			circular = false;

	
		var	dmp = function(object, pad){
			var	out = '',
				idt = '\t';

			circular = false;

			for (i = 0; i < visited.length; i++) {
				if (object === visited[i]) {
					circular = true;
					break;
				}
			}

			self.iteration++;
			if (self.iteration>self.maxIterations){
				return false;
			}

			pad = (pad === undefined) ? '' : pad;

			if (circular) {
				out = '[circular reference]';
			} 

			else if (object === null){
				out = 'null';
			} 

			else if (object != null && object != undefined){
				
				if (object.constructor == Array){
					out += '[';
					if (object.length>0){
						var arr = [];
						out += '\n';
						for (var i=0; i<object.length; i++){
							arr.push(pad + idt + dmp(object[i], pad + idt));
						}
						out += arr.join(',' + '\n') + '\n';
						out += pad;
					}
					out += ']';
				} 

				else if (object.constructor == Object){
					out += '{\n';
					visited.push(object);
					for (var i in object){
						out += pad + idt + i + ' : ' 
							+ dmp(object[i], pad + idt) + '\n';
					}
					out += pad + '}';
				} 

				else if (typeof(object) == "string"){
					out += '"' + object + '"';
				} 

				else if (typeof(object) == "number"){
					out += object.toString();
				} 

				else if (object.constructor === Function){
					visited.push(object);
					var source = object.toString();
					if (source.indexOf('[native code]') > -1) {
						out += "function(){ [native code] }";
					} else {
						out += "function(){ ... }"; //source;
					}

				} 

				else if (object.toString) {
					try {
						out += object;
					} catch(e){
						out += "function(){ [Native Code] }";
					}
				} else {
					out += "null";
				}
			} else {
				out += 'undefined';
			}
			return out;
		};

		self.iteration = 0;
		return dmp(object);
	}
};

/* -------------------------------------------------------------------------- */
/* -- Implement Object.isPrimitive() if not already built-in                  */
/* -------------------------------------------------------------------------- */

if (!("isPrimitive" in Object && typeof(Object.isPrimitive)==="function")) {
	Object.isPrimitive = function (o) o !== Object(o);
}

/* -------------------------------------------------------------------------- */
/* Default Forwarding Proxy Handler Implementation                            */
/* That proxy forwards all operations applied to it to an existing object     */
/* -------------------------------------------------------------------------- */

Object.Handler = function(obj){
	return {
		/* -- Fundamental Traps */

		defineProperty : function(key, descriptor){
			Object.defineProperty(obj, key, descriptor);
		},

		getOwnPropertyDescriptor : function(key){
			var descriptor = Object.getOwnPropertyDescriptor(obj, key);
			if (descriptor !== undefined) descriptor.configurable = true;
			return descriptor;
		},

		getPropertyDescriptor : function(key){
			var descriptor = Object.getOwnPropertyDescriptor(obj, key);
			if (descriptor !== undefined) descriptor.configurable = true;
			return descriptor;
		},

		getOwnPropertyNames : function(){
			return Object.getOwnPropertyNames(obj);
		},

		getPropertyNames : function(){
			return Object.getOwnPropertyNames(obj);
		},
		
		delete : function(key){
			return delete obj[key];
		},

		/* function calltrap */
		invoke : function(args){
			return obj.apply(this, args);
		},

		/* contructor trap */
		construct : function(args){
			var Forward = function(args){
				return obj.apply(this, args);
			};
			return new Forward(args);
		},

		fix : function(){
			if (Object.isFrozen(obj)){
				var result = {};
				Object.getOwnPropertyNames(obj).forEach(function(key){
					result[key] = Object.getOwnPropertyDescriptor(obj, key);
				});
				return result;
			}
			return undefined;
		},

		/* -- Derived Traps  */

		get : function(receiver, key){
			return obj[key];
		},

		set : function(receiver, key, val){
			obj[key] = val;
			return true;
		},

		has : function(key){
			return key in obj;
		},

		hasOwn : function(key){
			return ({}).hasOwnProperty.call(obj, key);
		},

		keys : function(){
			return Object.keys(obj);
		},

	 	enumerate : function(){
			var props = [];
			for (var key in obj){
				props.push(key);
			};
			return props;
		},

		iterate : function(){
			var idx = 0,
				props = this.enumerate(),
				size = +props.length;

			return {
				next : function(){
					if (idx === size) throw StopIteration;
					return props[idx++];
				}
			};

		}

	};
};

/* -------------------------------------------------------------------------- */
/* High Abstraction Identity-Preserving Membrane                              */
/* -------------------------------------------------------------------------- */
/* The following is a membrane implementation that satisfies the formal       */
/* property of unavoidable transitive interposition. The implementation       */
/* preserves the boundary between the two "wet" and "dry" sides.              */
/* -------------------------------------------------------------------------- */
/* More @ http://wiki.ecmascript.org/doku.php?id=harmony:proxies              */
/* -------------------------------------------------------------------------- */
/* Author : Vincent Fontaine                                                  */
/* -------------------------------------------------------------------------- */

Object.membrane = function(wetTarget, getForwardingHandler = Object.Handler){
	var wet2dry = new WeakMap(),
		dry2wet = new WeakMap();

	var getRevokeHandler = function(heatmap, wrap, mapper){
		var h = Proxy.create(Object.freeze({
			get : function(receiver, key){
				return function(...n){
					var handler = heatmap.get(h);
					try {
						return wrap(
							handler[key].apply(handler, n.map(mapper))
						);
					} catch (e) {
						var arg = "";
						if ("in" in Object){
							arg = key.in("get", "set") ? n[1] : "";
						}
						echo("Exception: not allowed", key, arg);
						throw wrap(e);
					}
				};
			}
		}));
		return h;
	};

	var createProxy = function(obj, heatmap, wrap, mapper, revokeHandler){
		var forwardHandler = getForwardingHandler(obj),
			revokeHandler = getRevokeHandler(heatmap, wrap, mapper);

		var callTrap = function(...n){
			return wrap(
				forwardHandler.invoke.call(mapper(this), n.map(mapper))
			);
		}

		var constructTrap = function(...n){
			return wrap(
				forwardHandler.construct(n.map(mapper))
			);
		}

		if (typeof obj === "function") {
			var proxy = Proxy.createFunction(
				revokeHandler, 
				callTrap, 
				constructTrap
			);
		} else {
			var proxy = Proxy.create(
				revokeHandler, 
				wrap(Object.getPrototypeOf(obj))
			);
		}

		heatmap.set(revokeHandler, forwardHandler);

		return proxy;
	};

	var getProxy = function(obj, side) {
		if (side == "wet") {
			var wrap = asWet,
				mapper = asDry,
				objmap = dry2wet,
				heatmap = wet2dry;
		} else {
			var wrap = asDry,
				mapper = asWet,
				objmap = wet2dry,
				heatmap = dry2wet;
		}

		var proxy = createProxy(obj, heatmap, wrap, mapper);

		objmap.set(obj, proxy);
		heatmap.set(proxy, obj);
		return proxy;
	};

	var asDry = function(obj){
		if (Object.isPrimitive(obj)) return obj;
		var proxy = wet2dry.get(obj);
		return proxy ? proxy : getProxy(obj, "dry");
	};

	var asWet = function(obj){
		if (Object.isPrimitive(obj)) return obj;
		var proxy = dry2wet.get(obj);
		return proxy ? proxy : getProxy(obj, "wet");
	};

	return Object.freeze({
		wrapper : asDry(wetTarget),
		revoke : function(){
			dry2wet = wet2dry = Object.freeze({
				get : function(key){
					throw "Read Access Denied. Membrane was revoked.";
				},

				set : function(key, val){
					throw "Write Access Denied. Membrane was revoked."; 
				}
			});
		}
	});
};


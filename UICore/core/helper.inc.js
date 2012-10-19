/* -------------------------- */
/* Native (@) 2012 Stight.com */
/* -------------------------- */

var window = {
	width : canvas.width,
	height : canvas.height,
	mouseX : 0,
	mouseY : 0
};

/* -------------------------------------------------------------------------- */

Math.distance = function(x1, y1, x2, y2){
	var a = y2-y1, b = x2-x1;
	return Math.sqrt(a*a+b*b);
};

/* -- The Catel Fontaine !n */
Math.factorial = (function(n){
	var c = [],
		f = function(n){
			if (n==0 || n==1) return 1;
			if (c[n]) return c[n];
			for (var r=i=1; i<=n; i++){r *= i}
			return c[n] = r;
		};
	for (var i=0; i<5000; i++) f(i);
	return f;
}());


/* -------------------------------------------------------------------------- */

canvas.implement = function(props){
	for (var key in props){
		if (props.hasOwnProperty(key)){
			canvas[key] = props[key];
			Canvas.prototype[key] = props[key];
		}
	}
};

canvas.implement({
	setColor : function(color){
		this.fillStyle = color;
	},

	setFontSize : function(fontSize){
		this.fontSize = fontSize;
	},

	setShadow : function(x, y, b, c){
		this.shadowOffsetX = x;
		this.shadowOffsetY = y;
		this.shadowColor = c;
		this.shadowBlur = b;
	}
});

/* -------------------------------------------------------------------------- */

Number.prototype.bound = function(min, max){
	return Math.min(Math.max(Number(min), this), Number(max));
};

/* Based on php.js -- http://phpjs.org/functions/number_format:481 */
Number.prototype.format = function(decimals, d, t){
	var number = (this + '').replace(/[^0-9+\-Ee.]/g, ''),
		n = !isFinite(+number) ? 0 : +number,
		prec = !isFinite(+decimals) ? 0 : Math.abs(decimals),
		sep = t === undefined ? ',' : t,
		dec = d === undefined ? '.' : d,
		s = '',
		toFixedFix = function (n, prec){
			var k = Math.pow(10, prec);
			return '' + Math.round(n * k) / k;
		};

	s = (prec ? toFixedFix(n, prec) : '' + Math.round(n)).split('.');
	if (s[0].length > 3){
		s[0] = s[0].replace(/\B(?=(?:\d{3})+(?!\d))/g, sep);
	}
	if ((s[1] || '').length < prec) {
		s[1] = s[1] || '';
		s[1] += new Array(prec - s[1].length + 1).join('0');
	}
	return s.join(dec);
};

/* -------------------------------------------------------------------------- */

String.prototype.mul = function(n){
	var st = [], m = Math.abs(Number(n));
	for (var t=0; t<m; t++){
		st.push(this);
	}
	return st.join('');
};

String.prototype.splice = function(offset, size, insert){
    return (
    	this.slice(0,offset) + 
    	OptionalString(insert, '') + 
    	this.slice(offset + Math.abs(size))
    );
};

String.prototype.htmlTrim = function(){
	return this.replace(/^[ \t\n\r\f]+|[ \t\n\r\f]+$/g, "");
}

/* -------------------------------------------------------------------------- */

var BenchThis = function(name, iterations, fn){
	var t = +new Date(),
		exec = 0,
		ips = 0,
		ipf = 0;

	for (var i = 0; i < iterations; i++){
		fn(i);
	}

	exec = (+new Date()-t);
	ips = Math.round(1000*iterations/exec);
	ipf = 16*iterations/exec;

	echo(name);

	echo(
		'  - '+iterations.format(0, '.', ' ')+' executions takes',
		exec.format(0, '.', ' '),
		"ms"
	);

	echo('  - Speed: '+ips.format(0, '.', ' ')+" exec/s");
	echo(
		'  - Oneshot ~ '+(exec/iterations).format(2, '.', ' ')+" ms", 
		"(max "+ipf.format(0, '.', ' ')+" exec/frame)"
	);

	echo('');
};

/* -------------------------------------------------------------------------- */

var OptionalString = function(x, def){
	return x === null || x === undefined ? String(def) : String(x);
}

var OptionalValue = function(x, def){
	return x === null || x === undefined ? def : x;
}

var OptionalNumber = function(x, def, min, max){
	var p = x === null || x === undefined ? Number(def) : Number(x);
	p = (min != undefined) ? Math.max(Number(min), p) : p;
	return (max != undefined) ? Math.min(Number(max), p) : p;
}

var OptionalBoolean = function(x, def){
	return x === null || x === undefined ? Boolean(def) : Boolean(x);
}

var OptionalCallback = function(x, def){
	return typeof x === "function" ? x : def;
}

/* -------------------------------------------------------------------------- */

/*  Debug Button
 * 
 *	Syntaxe: 	DBT(function(){
 *					Do Something On Click
 *				});
 * 
 */

var DBT = function(cb){
	if (!Native.layout.rootElement) return false;
	if (!this.DebugButton){
		this.DebugButton = Native.layout.rootElement.add("UIButton", {
			x : 970,
			y : 744,
			h : 16,
			label : "debug",
			color : "#000000",
			background : "#ff9900",
			radius : 2,
			fontSize : 10,
			lineHeight : 6
		});
	}
	this.DebugButton.addEventListener("mousedown", function(e){
		cb.call(this);
		e.stopPropagation();
	}, false);
};

/* -------------------------------------------------------------------------- */

var console = {
	iteration : 0,
	maxIterations : 20,

	log : function(...n){
		var m = [];
		for (var i in n){
			m.push(this.dump(n[i]));
		}
		echo.apply(this, m);
	},

	dump : function(object, pad){
		var self = this;
		
		var	dmp = function(object, pad){
			var	out = '',
				idt = '\t';

			self.iteration++;
			if (self.iteration>self.maxIterations){
				return false;
			}

			pad = (pad === undefined) ? '' : pad;

			if (object != null && object != undefined){
				if (object.constructor == Array){
					out += '[\n';
					for (var i=0; i<object.length; i++){
						out += pad + idt + dmp(object[i], pad + idt) + '\n';
					}
					out += pad + ']';
				} else if (object.constructor == Object){
					out += '{\n';
					for (var i in object){
						if (object.hasOwnProperty(i)) {
							out += pad + idt + i + ' : ' 
								+ dmp(object[i], pad + idt) + '\n';
						}
					}
					out += pad + '}';
				} else if (typeof(object) == "string"){
					out += '"' + object + '"';
				} else if (typeof(object) == "number"){
					out += object.toString();
				} else if (object.toString) {
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
		}

		self.iteration = 0;
		return dmp(object, pad);

	}
};

/* -------------------------------------------------------------------------- */

var setTimer = function(fn, ms, loop, execFirst){
	var t = {
		loop : loop,
		tid : loop 
			? setInterval(function(){fn.call(t);}, ms)
			: setTimeout(function(){fn.call(t);}, ms),

		remove : function(){
			if (this.loop) {
				clearInterval(this.tid);
			} else {
				clearTimeout(this.tid);
			}
			delete(this.tid);
		}
	};

	if (execFirst) {
		fn.call(t);
	}
	
	return t;
};

/* -------------------------------------------------------------------------- */

var FPS = {
	date : 0,
	count : 0,
	old : 0,

	start : function(){
		this.date = + new Date();
	},

	show : function(){
		var r = 0.1 + (+ new Date()) - this.date,
			fps = 1000/r;

		this.count++;

		if (this.count%30==0){
			this.old = Math.round((r-0.1)*10)/10;
		} 				
		
		canvas.setColor("#000000");
		canvas.fillRect(0, canvas.height-40, 60, 30);
		//canvas.fillRect(0, 280, 50, 30);
		canvas.setColor("yellow");
		canvas.fillText(this.old + " ms", 5, canvas.height-20);
		
		return r;
	}
};


/* -------------------------------------------------------------------------- */

Object.scope = this;

Object.append = function(o, prototype){
	if (typeof prototype !== "number" && typeof prototype !== "boolean"){
		var p = prototype,
			f = obj = last = o instanceof this ? o : new o.constructor(o),
			FP = Function.prototype,
			OP = Object.prototype,
			AP = Array.prototype,
			get = this.getPrototypeOf;

		for (var k = get(obj);	k !== OP && k !== FP; k = get(obj)){
			obj = k;
		}

		if (prototype.constructor === String){
			p = FP;
			f = Function.apply(null, AP.slice.call(arguments, 1));
			f.__proto__ = last;
		}

		obj.__proto__ = p;
		return f;
	}
}


/*
 *  JS Sourced Function (Sunction)
 *
 *  ---------
 *  + USAGE +
 *  ---------
 *
 *  var buddy = "dude";
 *  
 *  var f = new Sunction(
 *  	'var z = Math.random(),' + 
 *  	'	 o = arguments[0];' +
 *  
 *  	'this.foo = {' +
 *  	'	x : o,' +
 *  	'	y : buddy' +
 *  	'};'
 *  );
 * 
 *  f(48);
 *  
 */

var Sunction = function(source){
	return Object.append(new Function, source);
};

/* -------------------------------------------------------------------------- */


/* -- Implement Object.isPrimitive() if not already built-in */

if (!("isPrimitive" in Object && typeof(Object.isPrimitive)==="function")) {
	Object.isPrimitive = function (o) o !== Object(o);
}


/* 
 * Default Forwarding Proxy Handler Implementation
 * That proxy forwards all operations applied to it to an existing object
 */

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

		call : function(that, args){
			/* function call trap */
			return obj.apply(that, args);
		},

		construct : function(args){
			/* contructor trap */
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

		hasOwn : function(name){
			return ({}).hasOwnProperty.call(obj, name);
		},

		
		keys : function(){
			return Object.keys(obj);
		},

	 	enumerate : function(){
			var props = [];
			for (key in obj){
				props.push(key);
			};
			return props;
		}

	};
};


/*
 * Identity-Preserving Membrane
 * ----------------------------
 * The following is a membrane implementation that satisfies the formal property
 * of unavoidable transitive interposition. The implementation preserves the
 * boundary between the two "wet" and "dry" sides.
 *
 * More @ http://wiki.ecmascript.org/doku.php?id=harmony:proxies
 */

Object.Membrane = function(wetTarget, profiler){
	var wet2dry = new WeakMap(),
		dry2wet = new WeakMap();

	var getRevokeHandler = function(heat, source, mapper){
		var h = Proxy.create(Object.freeze({
			get : function(receiver, key){
				return function(...n){
					var handler = heat.get(h);
					try {
						return source(handler[key].apply(handler, n.map(mapper)));
					} catch(e) {
						throw source(e);
					}
				};
			}
		}));
		return h;
	};

	var getProxy = function(obj, signal){
		var source = (signal == "wet") ? asWet : asDry,
			mapper = (signal == "wet") ? asDry : asWet,

			heat1 = (signal == "wet") ? dry2wet : wet2dry,
			heat2 = (signal == "wet") ? wet2dry : dry2wet;

		var forwardingHandler = Object.Handler(obj),
			revokeHandler = getRevokeHandler(heat2, source, mapper);

		heat2.set(revokeHandler, forwardingHandler);

		var callTrap = function(...n){
			var k = revokeHandler.call(mapper(this), n.map(mapper));
			return source(k);
		};

		var constructorTrap = function(...n){
			var k = revokeHandler.construct(n.map(mapper));
			return source(k);
		};

		if (typeof obj === "function"){
			var proxy = Proxy.createFunction(
				revokeHandler, 
				callTrap,
				constructorTrap
			);
		} else {
			var proxy = Proxy.create(
				revokeHandler, 
				source(Object.getPrototypeOf(obj))
			);
		}

		heat1.set(obj, proxy);
		heat2.set(proxy, obj);

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

	var revoke = function(){
		dry2wet = wet2dry = Object.freeze({
			get : function(key){
				throw new Error("Access Denied. Membrane was revoked.");
			},

			set : function(key, val){

			}
		});
	};

	return Object.freeze({
		wrapper : asDry(wetTarget),
		revoke : Object.freeze(revoke)
	});

};


/* -------------------------------------------------------------------------- */

/*
 *  CStruct Fast Buffer
 *
 *  var s = new CStruct(
 *  	"unsigned long id",
 *  	"char username[16]",
 *  	"float amountDue;"
 *  );
 *  
 *  var view = new Float32Array(s.buffer);
 *  
 *  for (var i=0; i<6; i++){
 *  	echo("buffer : " + view[i]);
 *  }
 *  
 *  s.amountDue[0] = 158.5;
 *  
 *  for (var i=0; i<6; i++){
 *  	echo("buffer : " + view[i]);
 *  }
 *  
 */


var CStruct = function(){
	var seek = 0,
		shader = [],
		types = {
			"char" : {m:Int8Array, l:1},
			"signed char" : {m:Int8Array, l:1},
			"unsigned char" : {m:Uint8Array, l:1},

			"short" : {m:Int16Array, l:2},
			"unsigned short" : {m:Uint16Array, l:2},

			"int" : {m:"Int32Array", l:4},
			"unsigned int" : {m:Uint32Array, l:4},
			"unsigned long" : {m:Uint32Array, l:4},

			"float" : {m:Float32Array, l:4},
			"double" : {m:Float64Array, l:8},
			"double int" : {m:Float64Array, l:8}
		};

	this.size = 0;
	this.seek = 0;
	this.buffer = null;

	for (var a in arguments){
		var c = arguments[a].replace(";", ""),
			s = c.split(" "), i = s.length-1,
			t = (c.replace(" " + s[i], "")).toLowerCase(),
			n = s[i].split("["), name = (n[0]),
			f = n[1] ? (n[1].split("]"))[0] : null,
			size = f ? f : 1;

		if (types[t]) {
			shader.push({
				name : name,
				fn : types[t].m,
				seek : seek,
				size : size
			});
			seek += types[t].l * size;
		}
	}

	if (seek!=0) {
		this.size = seek;
		this.buffer = new ArrayBuffer(this.size);
		for (var i in shader){
			var name = shader[i].name,
				fn = shader[i].fn;
			this[name] = new fn(this.buffer, shader[i].seek, shader[i].size);
		}
	}
	return this;
};








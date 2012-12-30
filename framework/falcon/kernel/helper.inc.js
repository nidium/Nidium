/* -------------------------- */
/* Native (@) 2013 Stight.com */
/* -------------------------- */

Object.merge = function(obj, props){
	obj = obj || {};
	for (var key in props){
		if (props.hasOwnProperty(key)){
			obj[key] = props[key];
		}
	}
};

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

Canvas.prototype.clear = function(){
	var context = this.getContext("2D");
	context.clearRect(
		0, 
		0, 
		this.width,
		this.height
	);

	if (Native.__debugger && this.host && this.host._root == Native.__debugger){
		return false;
	}
	
	if (__DEBUG_SHOW_LAYERS__) {
		if (this.host && this.host.hover){
			context.fillStyle = "rgba(180, 180, 50, 0.1)";
			context.strokeStyle = "rgba(180, 180, 255, 0.3)";
		} else {
			context.fillStyle = "rgba(180, 180, 0, 0.01)";
			context.strokeStyle = "rgba(180, 180, 0, 0.05)";
		}

		context.fillRect(
			0, 
			0, 
			this.width,
			this.height
		);

		context.strokeRect(
			0, 
			0, 
			this.width,
			this.height
		);
	}

	if (__DEBUG_SHOW_ORDER__ === true) {
		context.fontSize = 9;
		if (this.host && this.host._hover){
			context.setText(
				this.host._nid,
				this.host._width + 7,
				9,
				"white",
				"rgba(0, 0, 0, 0.7)"
			);
		} else {
			context.setText(
				this.host._nid,
				this.host._width + 7,
				9,
				"rgba(200, 0, 0, 0.8)",
				"rgba(0, 0, 0, 0.7)"
			);
		}
	}

};

/* -------------------------------------------------------------------------- */

Number.prototype.bound = function(min, max){
	return Math.min(Math.max(Number(min), this), Number(max));
};

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

var console = {
	iteration : 0,
	maxIterations : 20,

	log : function(...n){
		echo.apply(this, n.map(this.dump));
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
		}

		self.iteration = 0;
		return dmp(object);

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

Native.FPS = {
	date : 0,
	count : 0,
	old : 0,

	init : function(){
		this.canvas = new Canvas(50, 30);
		this.context = this.canvas.getContext("2D");
		this.canvas.left = 0;
		this.canvas.top = window.height-30;
		Native.canvas.add(this.canvas);
		this.loaded = true;
	},

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
		
		if (this.loaded) {
			this.context.setColor("#000000");
			this.context.fillRect(0, 0, 60, 30);
			this.context.setColor("yellow");
			this.context.fillText(this.old + " ms", 8, 20);
		}
		
		return r;
	}
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
	for (var i=0; i<5000; i++) f(i);
	return f;
}());

/* -------------------------------------------------------------------------- */

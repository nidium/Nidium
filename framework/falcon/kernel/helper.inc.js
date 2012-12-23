/* -------------------------- */
/* Native (@) 2013 Stight.com */
/* -------------------------- */

var window = {
	width : Native.canvas.width,
	height : Native.canvas.height,
	mouseX : 0,
	mouseY : 0,
	requestAnimationFrame : Native.canvas.getContext("2D").requestAnimationFrame
};

/* -------------------------------------------------------------------------- */

Canvas.prototype.clear = function(){
	this.getContext("2D").clearRect(
		0, 
		0, 
		this.width,
		this.height
	);
	/*
	this.getContext("2D").fillStyle = "rgba(180, 180, 0, 0.05)";
	this.getContext("2D").fillRect(
		0, 
		0, 
		this.width,
		this.height
	);

	this.getContext("2D").strokeStyle = "rgba(180, 180, 0, 0.3)";
	this.getContext("2D").strokeRect(
		0, 
		0, 
		this.width,
		this.height
	);
	*/
};

/* -------------------------------------------------------------------------- */

Number.prototype.bound = function(min, max){
	return Math.min(Math.max(Number(min), this), Number(max));
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

var __fpsLayer = new Canvas(50, 30),
	__fpsLayerContext = __fpsLayer.getContext("2D");

__fpsLayer.left = 0;
__fpsLayer.top = window.height-40;
Native.canvas.add(__fpsLayer);

var FPS = {
	date : 0,
	count : 0,
	old : 0,

	start : function(){
		this.date = + new Date();
	},

	show : function(){
		var context = __fpsLayerContext,
			r = 0.1 + (+ new Date()) - this.date,
			fps = 1000/r;

		this.count++;

		if (this.count%30==0){
			this.old = Math.round((r-0.1)*10)/10;
		} 				
		
		context.setColor("#000000");
		context.fillRect(0, 0, 60, 30);
		context.setColor("yellow");
		context.fillText(this.old + " ms", 8, 20);
		
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

/*
 * Open source under the BSD License. 
 * Copyright (c) 2008 George McGinley Smith
 */

// t: current time, b: beginning value, c: change In value, d: duration

Math.physics = {
	def: 'quadOut',
	
	swing: function (x, t, b, c, d) {
		return Math.physics[Math.physics.def](x, t, b, c, d);
	},
	
	/* -- QUAD */
	quadIn: function (x, t, b, c, d) {
		return c*(t/=d)*t + b;
	},
	
	quadOut: function (x, t, b, c, d) {
		return -c *(t/=d)*(t-2) + b;
	},
	
	quadInOut: function (x, t, b, c, d) {
		if ((t/=d/2) < 1) return c/2*t*t + b;
		return -c/2 * ((--t)*(t-2) - 1) + b;
	},
	
	/* -- CUBIC */
	cubicIn: function (x, t, b, c, d) {
		return c*(t/=d)*t*t + b;
	},
	
	cubicOut: function (x, t, b, c, d) {
		return c*((t=t/d-1)*t*t + 1) + b;
	},
	
	cubicInOut: function (x, t, b, c, d) {
		if ((t/=d/2) < 1) return c/2*t*t*t + b;
		return c/2*((t-=2)*t*t + 2) + b;
	},
	
	/* -- QUART */
	quartIn: function (x, t, b, c, d) {
		return c*(t/=d)*t*t*t + b;
	},
	
	quartOut: function (x, t, b, c, d) {
		return -c * ((t=t/d-1)*t*t*t - 1) + b;
	},
	
	quartInOut: function (x, t, b, c, d) {
		if ((t/=d/2) < 1) return c/2*t*t*t*t + b;
		return -c/2 * ((t-=2)*t*t*t - 2) + b;
	},
	
	/* -- QUINT */
	quintIn: function (x, t, b, c, d) {
		return c*(t/=d)*t*t*t*t + b;
	},
	
	quintOut: function (x, t, b, c, d) {
		return c*((t=t/d-1)*t*t*t*t + 1) + b;
	},
	
	quintInOut: function (x, t, b, c, d) {
		if ((t/=d/2) < 1) return c/2*t*t*t*t*t + b;
		return c/2*((t-=2)*t*t*t*t + 2) + b;
	},

	/* -- Sinusoid */	
	sineIn: function (x, t, b, c, d) {
		return -c * Math.cos(t/d * (Math.PI/2)) + c + b;
	},
	
	sineOut: function (x, t, b, c, d) {
		return c * Math.sin(t/d * (Math.PI/2)) + b;
	},
	
	sineInOut: function (x, t, b, c, d) {
		return -c/2 * (Math.cos(Math.PI*t/d) - 1) + b;
	},
	
	/* -- exponential */
	expoIn: function (x, t, b, c, d) {
		return (t==0) ? b : c * Math.pow(2, 10 * (t/d - 1)) + b;
	},
	
	expoOut: function (x, t, b, c, d) {
		return (t==d) ? b+c : c * (-Math.pow(2, -10 * t/d) + 1) + b;
	},
	
	expoInOut: function (x, t, b, c, d) {
		if (t==0) return b;
		if (t==d) return b+c;
		if ((t/=d/2) < 1) return c/2 * Math.pow(2, 10 * (t - 1)) + b;
		return c/2 * (-Math.pow(2, -10 * --t) + 2) + b;
	},
	
	/* -- circular */
	circIn: function (x, t, b, c, d) {
		return -c * (Math.sqrt(1 - (t/=d)*t) - 1) + b;
	},
	
	circOut: function (x, t, b, c, d) {
		return c * Math.sqrt(1 - (t=t/d-1)*t) + b;
	},
	
	circInOut: function (x, t, b, c, d) {
		if ((t/=d/2) < 1) return -c/2 * (Math.sqrt(1 - t*t) - 1) + b;
		return c/2 * (Math.sqrt(1 - (t-=2)*t) + 1) + b;
	},
	
	/* -- Elastic */
	elasticIn: function (x, t, b, c, d) {
		var s=1.70158;var p=0;var a=c;
		if (t==0) return b;  if ((t/=d)==1) return b+c;  if (!p) p=d*.3;
		if (a < Math.abs(c)) { a=c; var s=p/4; }
		else var s = p/(2*Math.PI) * Math.asin (c/a);
		return -(a*Math.pow(2,10*(t-=1)) * Math.sin( (t*d-s)*(2*Math.PI)/p )) + b;
	},
	
	elasticOut: function (x, t, b, c, d) {
		var s=1.70158;var p=0;var a=c;
		if (t==0) return b;  if ((t/=d)==1) return b+c;  if (!p) p=d*.3;
		if (a < Math.abs(c)) { a=c; var s=p/4; }
		else var s = p/(2*Math.PI) * Math.asin (c/a);
		return a*Math.pow(2,-10*t) * Math.sin( (t*d-s)*(2*Math.PI)/p ) + c + b;
	},

	elasticInOut: function (x, t, b, c, d) {
		var s=1.70158;var p=0;var a=c;
		if (t==0) return b;  if ((t/=d/2)==2) return b+c;  if (!p) p=d*(.3*1.5);
		if (a < Math.abs(c)) { a=c; var s=p/4; }
		else var s = p/(2*Math.PI) * Math.asin (c/a);
		if (t < 1) return -.5*(a*Math.pow(2,10*(t-=1)) * Math.sin( (t*d-s)*(2*Math.PI)/p )) + b;
		return a*Math.pow(2,-10*(t-=1)) * Math.sin( (t*d-s)*(2*Math.PI)/p )*.5 + c + b;
	},
	
	/* -- Back */
	backIn: function (x, t, b, c, d, s) {
		if (s == undefined) s = 1.70158;
		return c*(t/=d)*t*((s+1)*t - s) + b;
	},
	
	backOut: function (x, t, b, c, d, s) {
		if (s == undefined) s = 1.70158;
		return c*((t=t/d-1)*t*((s+1)*t + s) + 1) + b;
	},
	
	backInOut: function (x, t, b, c, d, s) {
		if (s == undefined) s = 1.70158; 
		if ((t/=d/2) < 1) return c/2*(t*t*(((s*=(1.525))+1)*t - s)) + b;
		return c/2*((t-=2)*t*(((s*=(1.525))+1)*t + s) + 2) + b;
	},
	
	/* -- Bounce */
	bounceIn: function (x, t, b, c, d) {
		return c - Math.physics.bounceOut(x, d-t, 0, c, d) + b;
	},
	
	bounceOut: function (x, t, b, c, d) {
		if ((t/=d) < (1/2.75)) {
			return c*(7.5625*t*t) + b;
		} else if (t < (2/2.75)) {
			return c*(7.5625*(t-=(1.5/2.75))*t + .75) + b;
		} else if (t < (2.5/2.75)) {
			return c*(7.5625*(t-=(2.25/2.75))*t + .9375) + b;
		} else {
			return c*(7.5625*(t-=(2.625/2.75))*t + .984375) + b;
		}
	},
	
	bounceInOut: function (x, t, b, c, d) {
		if (t < d/2) return Math.physics.bounceIn(x, t*2, 0, c, d) * .5 + b;
		return Math.physics.bounceOut(x, t*2-d, 0, c, d) * .5 + c*.5 + b;
	}
};
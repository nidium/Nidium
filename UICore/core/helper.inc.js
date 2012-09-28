/* -------------------------- */
/* Native (@) 2012 Stight.com */
/* -------------------------- */

Math.distance = function(x1, y1, x2, y2){
	var a = (y2 - y1),
		b = (x2 - x1);
	return Math.sqrt(a*a+b*b);
};

/* ------------------------------------ */

var window = {
	width : canvas.width,
	height : canvas.height,
	mouseX : 0,
	mouseY : 0
};

canvas.implement = function(props){
	for (var key in props){
		if (props.hasOwnProperty(key)){
			canvas[key] = props[key];
			Canvas.prototype[key] = props[key];
		}
	}
};

canvas.implement({
	setShadow : function(x, y, b, c){
		this.shadowOffsetX = x;
		this.shadowOffsetY = y;
		this.shadowColor = c;
		this.shadowBlur = b;
	}
});

//var ___call = 0;
canvas.implement({
	currentColor : '',
	currentFontSize : '',
	setColor : function(color){
		if (this.currentColor == color) return false;
		this.currentColor = color;
		this.fillStyle = color;
	},
	setFontSize : function(fontSize){
		if (this.currentFontSize == fontSize) return false;
		this.currentFontSize = fontSize;
		this.fontSize = fontSize;
//		___call++;
	}
});

/*
setInterval(function(){
	echo(___call);
	___call = 0;
}, 1000);
*/

Number.prototype.bound = function(min, max){
	return Math.min(Math.max(Number(min), this), Number(max));
};

String.prototype.splice = function(offset, size, insert){
    return (this.slice(0,offset) + OptionalString(insert, '') + this.slice(offset + Math.abs(size)));
};

String.prototype.htmlTrim = function(){
	return this.replace(/^[ \t\n\r\f]+|[ \t\n\r\f]+$/g, "");
}

function OptionalString(x, def) {
	return x === null || x === undefined ? String(def) : String(x);
}

function OptionalBoolean(x, def) {
	return x === null || x === undefined ? Boolean(def) : Boolean(x);
}

function OptionalCallback(x, def) {
	return typeof x === "function" ? x : def;
}


var console = {
	iteration : 0,
	maxIterations : 20,

	log : function(message){
		if (typeof message == 'object'){
			echo(this.dump(message));
		} else {
			echo(message);
		}
	},

	dump : function(object, pad){
		var self = this;
		var dmp = function(object, pad){
			var	out = '',
				indent = '\t';

			self.iteration++;
			if (self.iteration>self.maxIterations){
				return false;
			}

			pad = (!pad) ? '' : pad;

			if (object != null && typeof(object) != "undefined") {
				if (object.constructor == Array){
					out += '[\n';
					for (var i=0; i<object.length; i++){
						out += pad + indent + dmp(object[i], pad + indent) + '\n';
					}
					out += pad + ']';
				} else if (object.constructor == Object){
					out += '{\n';
					for (var i in object){
						if (object.hasOwnProperty(i)) {
							out += pad + indent + i + ' : ' + dmp(object[i], pad + indent) + '\n';
						}
					}
					out += pad + '}';
				} else if (typeof(object) == "string"){
					out += '"' + object + '"';
				} else if (typeof(object) == "number"){
					out += object.toString();
				} else {
					out += object;
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

var count = function(arr){
	var len = 0;
	for (var i in arr){
		if (arr.hasOwnProperty(i)) {
			len++;
		}
	}
	return len;
};

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
		canvas.fillRect(0, 280, 50, 30);
		canvas.setColor("yellow");
		canvas.fillText(this.old + " ms", 5, canvas.height-20);

		return r;
	}
};

/* ----------------------- */

var CStruct = function(){
	var seek = 0,
		shader = [],
		types = {
			"char" : {m:"Int8Array", l:1},
			"signed char" : {m:"Int8Array", l:1},
			"unsigned char" : {m:"Uint8Array", l:1},

			"short" : {m:"Int16Array", l:2},
			"unsigned short" : {m:"Uint16Array", l:2},

			"int" : {m:"Int32Array", l:4},
			"unsigned int" : {m:"Uint32Array", l:4},
			"unsigned long" : {m:"Uint32Array", l:4},

			"float" : {m:"Float32Array", l:4},
			"double int" : {m:"Float64Array", l:8}
		};

	this.size = 0;
	this.buffer = null;

	for (var a in arguments){
		var c = arguments[a].replace(";", ""),
			s = c.split(" "), i = s.length-1,
			t = (c.replace(" " + s[i], "")).toLowerCase(),
			n = s[i].split("["), name = (n[0]).toLowerCase(),
			f = n[1] ? (n[1].split("]"))[0] : null,
			size = f ? f : 1;

		if (types[t]) {
			shader.push("this."+name+" = new "+types[t].m+"(this.buffer, "+seek+", "+size+");\n");
			seek += types[t].l * size;
		}
	}

	if (seek!=0) {
		this.size = seek;
		this.buffer = new ArrayBuffer(this.size);
		eval(shader.join(""));
	}
};
CStruct.prototype = {
	seek : 0,
	buffer : null
};

/*

var s = new Struct(
	"unsigned long id",
	"char username[16]",
	"float amountDue;"
);

var access = new Float32Array(s.buffer);

for (var i=0; i<6; i++){
	echo("buffer : " + access[i]);
}

s.amountDue[0] = 158.5;

for (var i=0; i<6; i++){
	echo("buffer : " + access[i]);
}

*/








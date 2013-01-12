/* -------------------------------------------------------------------------- */
/* Some Helper Functions                                                      */
/* -------------------------------------------------------------------------- */
/* Author : Vincent Fontaine                                                  */
/* -------------------------------------------------------------------------- */

"use strict";

/* -------------------------------------------------------------------------- */

Object.dumpTemplate = function(){
	var	buffer = this,
		size = buffer.byteLength,
		bytes = Array.prototype.slice.call(new Uint8Array(buffer)),
		ascii = "",
		hex = "",
		output = [],
		items = 22,
		offset = 0,
		x = 0;

	for (var i=0; i<size; i++){
		x++;
		hex += (bytes[i].toString(16)).leftPad(2, "0") + " ";
		ascii += String.fromCharCode(bytes[i]).replace(/[\x00-\x1F]/g, ".");

		if (x>=items) {
			x = 0;
			output.push((offset.toString()).leftPad(10, "0")+" : "+hex+"| "+ascii+"\n");
			hex = "";
			ascii = "";
			offset += items;
		}
	}

	if (x!=0){
		for (var i=x; i<items; i++){
			hex += '-- ';
		}
		output.push((offset.toString()).leftPad(10, "0")+" : "+hex+"| "+ascii);
	}
	echo(output.join(''));
};

ArrayBuffer.prototype.dump = Object.dumpTemplate;
Uint8Array.prototype.dump = Object.dumpTemplate;
String.prototype.dump = function(){
	var size = this.length,
		buffer = new ArrayBuffer(size),
		view = new Uint8Array(buffer);

	for (var i=0; i<size; i++) {
		view[i] = this.charCodeAt(i);
	}
	buffer.dump();
};

var malloc = function(bytes){
	var buffer = new ArrayBuffer(bytes);
	buffer.data = new Uint8Array(buffer);
	return buffer;
}

/* -------------------------------------------------------------------------- */

/*
 * Courtesy of Jon Leighton
 * Converts an ArrayBuffer directly to base64, without any intermediate
 * 'convert to string then use btoa' step.
 * This appears to be a faster approach:
 * http://jsperf.com/encoding-xhr-image-data/5
 */
Object.toBase64Template = function(){
	var base64 = '',
		a, b, c, d,
		e = 'ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/',
		bytes         = new Uint8Array(this),
		byteLength    = bytes.byteLength,
		byteRemainder = byteLength % 3,
		mainLength    = byteLength - byteRemainder,
		chunk;
 
	for (var i = 0; i < mainLength; i = i + 3){
		chunk = (bytes[i] << 16) | (bytes[i + 1] << 8) | bytes[i + 2];
		a = (chunk & 16515072) >> 18; // 16515072 = (2^6 - 1) << 18
		b = (chunk & 258048)   >> 12; // 258048   = (2^6 - 1) << 12
		c = (chunk & 4032)     >>  6; // 4032     = (2^6 - 1) << 6
		d = chunk & 63;               // 63       = 2^6 - 1
		base64 += e[a] + e[b] + e[c] + e[d];
	}
 
	if (byteRemainder == 1){
		chunk = bytes[mainLength];
		a = (chunk & 252) >> 2; // 252 = (2^6 - 1) << 2
		b = (chunk & 3)   << 4; // 3   = 2^2 - 1
		base64 += e[a] + e[b] + '==';
	} else if (byteRemainder == 2){
		chunk = (bytes[mainLength] << 8) | bytes[mainLength + 1];
		a = (chunk & 64512) >> 10; // 64512 = (2^6 - 1) << 10
		b = (chunk & 1008)  >>  4; // 1008  = (2^6 - 1) << 4
		c = (chunk & 15)    <<  2; // 15    = 2^4 - 1
		base64 += e[a] + e[b] + e[c] + '=';
	}
	return base64;
};
ArrayBuffer.prototype.toBase64 = Object.toBase64Template;

/* -------------------------------------------------------------------------- */

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

var console = {
	iteration : 0,
	maxIterations : 20,

	log : function(...n){
		for (var i in n){
			echo.call(this, this.dump(n[i]));
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




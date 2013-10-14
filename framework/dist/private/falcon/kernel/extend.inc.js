/* ------------------------+------------- */
/* Native Framework 2.0    | Falcon Build */
/* ------------------------+------------- */
/* (c) 2013 nidium.com - Vincent Fontaine */
/* -------------------------------------- */

"use strict";

/* -------------------------------------------------------------------------- */
/* IMAGE API                                                                  */
/* -------------------------------------------------------------------------- */

Image.load = function(url, callback){
	var cb = OptionalCallback(callback, function(){}),
		img = new Image();

	img.onload = function(){
		cb(img);
	};
	img.src = url;
};

/* -------------------------------------------------------------------------- */
/* HTTP API                                                                   */
/* -------------------------------------------------------------------------- */

var HttpRequest = function(method, url, data, onloadCallback){
	var self = this;
	this.h = new Http(url);

	this.h.ondata = function(e){
		if (typeof self.ondata == "function"){
			var percent = Number(e.total) !=0 ? Number(e.read)*100/e.total : 0;
			e.size = (e.type == "binary" ? e.data.byteLength : e.data.length);
			e.percent = Math.round(percent*100)/100;
			self.ondata.call(this, e);
		}
	};

	this.h.onerror = function(e){
		if (typeof self.onerror == "function"){
			self.onerror.call(this, e);
		}
	};
	
	this.h.request({
		headers: {
			"User-Agent": window.navigator.userAgent,
		},
		method : method,
		data : data
	}, function(e){
		if (typeof onloadCallback == "function"){
			onloadCallback.call(this, e);
		}
	});

	this.close = function(){
		this.h = null;
		delete this.h;
	};
};

/* -------------------------------------------------------------------------- */
/* FILE API                                                                   */
/* -------------------------------------------------------------------------- */

Object.merge(File.prototype, {
	offset : 0,

	load : function(callback){
		if (typeof callback != "function" && typeof this.onload != "function"){
			throw "File read : callback parameter or onload method expected";
		}

		callback = callback ? callback : this.onload;

		if (this.mutex) {
			throw new Error("File locked until callback");
			return false;
		}

		this.mutex = true;

		this.open(function(){
			this.read(5000000, function(buffer){
				this.mutex = false;
				this.offset = 0;
				this.size = buffer.byteLength;
				this.buffer = buffer;
				callback.call(this, this.buffer, this.size);
			});
		});
	},

	get : function(...n){
		var view = this.buffer,
			offset = 0,
			size = 0,
			maxSize = 0;

		if (!this.buffer) {
			throw new Error("File read : can't access uninitialized file");
		}

		switch (n.length){
			case 1 :
				/* f.read(size) */
				offset = this.offset;
				size = OptionalNumber(n[0], null);
				break;

			case 2 :
				/* f.read(offset, size) */
				offset = OptionalNumber(n[0], 0);
				size = OptionalNumber(n[1], null);
			 	break;

			 default :
				throw new Error("File read : missing parameters");
			 	break;
		}

		if (size == null){
			throw new Error("File read : expected size");
		}

		offset = offset.bound(0, this.size-1);
		maxSize = this.size-offset;

		size = size == null ? maxSize : size;
		size = size.bound(1, maxSize);
		
		if (offset == 0 && size == this.size){
			view = this.buffer;
		} else {
			view = new Uint8Array(this.buffer, offset, size);
		}

		this.offset = offset + size;
		return view;
	}
});

File.getText = function(url, callback){
	var f = new File(url);
	f.open("r", function(){
		f.read(f.filesize, function(buffer){
			this.size = buffer.byteLength;
			this.buffer = buffer;
			if (typeof callback == "function"){
				callback.call(this, this.buffer.toString());
			}
		});
	});
};

File.read = function(url, callback){
	var f = new File(url);
	f.open("r", function(){
		f.read(f.filesize, function(buffer){
			this.size = buffer.byteLength;
			this.buffer = buffer;
			if (typeof callback == "function"){
				callback.call(this, this.buffer, this.size);
			}
		});
	});
};

File.write = function(url, data, callback){
	var f = new File(url);
	f.open("w", function(){
		f.write(data, function(){
			this.close();
			if (typeof callback == "function"){
				callback.call(this);
			}
		});
	});
};

File.append = function(url, data, callback){
	var f = new File(url);
	f.open("a", function(){
		f.write(data, function(){
			this.close();
			if (typeof callback == "function"){
				callback.call(this);
			}
		});
	});
};

/* -------------------------------------------------------------------------- */
/* CONSOLE API                                                                */
/* -------------------------------------------------------------------------- */

console.dump = function(...n){
	this.iteration = 0;
	this.maxIterations = 5000;
	for (var i in n){
		if (n.hasOwnProperty(i)) {
			console.log(console.parse(n[i]));
		}
	}
};

console.parse = function(object){
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
};

/* -------------------------------------------------------------------------- */


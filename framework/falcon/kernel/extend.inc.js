/* ------------------------+------------- */
/* Native Framework 2.0    | Falcon Build */
/* ------------------------+------------- */
/* (c) 2013 Stight.com - Vincent Fontaine */
/* -------------------------------------- */

"use strict";

/* -------------------------------------------------------------------------- */

var HttpRequest = function(url, onloadCallback){
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
	
	this.h.request(function(e){
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
			if (typeof callback == "function") callback.call(this, this.buffer.toString());
		});
	});
};

File.read = function(url, callback){
	var f = new File(url);
	f.open("r", function(){
		f.read(f.filesize, function(buffer){
			this.size = buffer.byteLength;
			this.buffer = buffer;
			if (typeof callback == "function") callback.call(this, this.buffer, this.size);
		});
	});
};

File.write = function(url, data, callback){
	var f = new File(url);
	f.open("w", function(){
		f.write(data, function(){
			this.close();
			if (typeof callback == "function") callback.call(this);
		});
	});
};

File.append = function(url, data, callback){
	var f = new File(url);
	f.open("a", function(){
		f.write(data, function(){
			this.close();
			if (typeof callback == "function") callback.call(this);
		});
	});
};

/* -------------------------------------------------------------------------- */


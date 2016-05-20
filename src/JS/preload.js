/* -------------------------------------------------------------------------- */
/* MIT license                                          (c) 2016 Nidium, Inc. */
/* -------------------------------------------------------------------------- */
/* Permission is hereby granted, free of charge, to any person obtaining a    */
/* copy of this software and associated documentation files (the "Software"), */
/* to deal in the Software without restriction, including without limitation  */
/* the rights to use, copy, modify, merge, publish, distribute, sublicense,   */
/* and/or sell copies of the Software, and to permit persons to whom the      */
/* Software is furnished to do so, subject to the following conditions:       */
/*                                                                            */
/* The above copyright notice and this permission notice shall be included in */
/* all copies or substantial portions of the Software.                        */
/*                                                                            */
/* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR */
/* IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,   */
/* FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL    */
/* THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER */
/* LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING    */
/* FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER        */
/* DEALINGS IN THE SOFTWARE.                                                  */
/* -------------------------------------------------------------------------- */

"use strict";

/* -------------------------------------------------------------------------- */

Debug.RemoteMessage = function(instcount, data, wrapper) {
	this.data = data;
	this.instcount = instcount;
	this.wrapper = wrapper;
};

Debug.RemoteMessage.prototype.reply = function(data) {
	this.wrapper.send(data, null, this.instcount);
};

Debug.RemoteMessage.prototype.getData = function() {
	return this.data;
};

/* -------------------------------------------------------------------------- */

Debug.RemoteMessageHandler = function(){
	this.reset();
};

Debug.RemoteMessageHandler.prototype = {
	onmessage : function(){},

	read: function(data, offset = 0) {
		const uint32_size = Uint32Array.BYTES_PER_ELEMENT;

		if (!this.buffering) {
			var header 	   = new Uint32Array(data, offset, 1);
			this.remaining = header[0];
			this.buffering = true;
			this.buffer    = new Uint8Array(this.remaining);
			this.buffer_offset = 0;

			return this.read(data, uint32_size + offset);
		}

		var content = new Uint8Array(data, offset);
		var leftover = content.length - this.remaining;
		var sub = content.subarray(0, this.remaining);
		this.buffer.set(sub, this.buffer_offset);

		this.buffer_offset += sub.length;
		this.remaining -= sub.length;

		if (this.remaining === 0) {
			var obj = Debug.unserialize(this.buffer.buffer);
			this.onmessage(obj);
			this.buffering = 0;
			if (leftover) {
				this.read(data, offset + sub.length);
			}
		}
	},

	reset : function() {
		this.buffering = false;
		this.buffer = null;
		this.remaining = 0;
		this.offset = 0;
	}
};

/* -------------------------------------------------------------------------- */

Debug.state = {
	connected : false,
	timer : null,
	start : false,
	socket : null,
	remote : new Debug.RemoteMessageHandler(),
	instructionCount : 1,
	callbacks : new Map()
};

Debug.onmessage = function(){};
Debug.onready = function(){};

/* -------------------------------------------------------------------------- */

Debug.start = function(server = "127.0.0.1", port=2712)	{
	var self = this,
		ds = Debug.state;

	if (ds.start) return;
	ds.start = true;

	console.log("Debug Session Started");

	var client = new Socket(server, port).connect();
	client.binary = true;

	client.onconnect = function(){
		setTimeout(function(){
			self.onready.call(self);
			console.log("-- connected to Studio");
		}, 1);
		ds.connected = true;
	};

	client.ondisconnect = function(){
		ds.remote.reset();
		ds.connected = false;
		if (ds.start) {
			ds.timer = setTimeout(function(){
				client.connect();
			}, 500);
		}
	};

	client.onread = function(data){
		ds.remote.read(data);
	};

	ds.socket = client;
};

/* -------------------------------------------------------------------------- */

Debug.stop = function()	{
	Debug.state.start = false;
	clearTimeout(Debug.state.timer);

	if (Debug.state.socket){
		Debug.state.socket.disconnect();
	}
};

/* -------------------------------------------------------------------------- */

Debug.send = function(obj, callback = null, isReply = null)	{
	var ds = Debug.state;
	if (!ds.connected) return;

	var inst = {
		ic: ds.instructionCount++,
		obj: obj,
		isReply: isReply
	};

	if (callback) {
		ds.callbacks.set(inst.ic, callback);
	}

	var data = Debug.serialize(inst);

	var header = new Uint32Array(1);
	header[0] = data.byteLength;
	
	ds.socket.write(header.buffer);
	ds.socket.write(data);
};

/* -------------------------------------------------------------------------- */

Debug.state.remote.onmessage = function(obj) {
	var ds = Debug.state;

	if (obj.isReply) {
		var cb = ds.callbacks.get(obj.isReply);
		if (cb) {
			cb(obj.obj);
			ds.callbacks.delete(obj.isReply);
		}
	} else {
		var msg = new Debug.RemoteMessage(obj.ic, obj.obj, Debug);
		Debug.onmessage(msg);
	}
};

/* -------------------------------------------------------------------------- */

Debug.onmessage = function(ev) {
	var message = ev.getData();

	switch(message.action) {
		case 'exec':
			var args = message.args || [];
			ev.reply(message.data.apply(this, args));
			break;

		case 'eval':
			var ret;
			try {
				ret = eval(message.data);
			} catch(e) {
				ret = e.toString();
			}
			ev.reply(ret);
			break;
	}
};

/* -------------------------------------------------------------------------- */

Debug.ProxyHandler = {
	//__wrappedObject: null,
	get : function(target, name) {
		if (name == "__isProxy") {
			return true;
		}

		if (name == "__wrappedObject") {
			return target;
		}

		/*
			if (!(name in target)) {
			var p = new Proxy({}, Debug.ProxyHandler);
			target[name] = p;

			return target[name];
		}*/

		if (!target[name] || target[name].__isProxy) {
			return target[name];
		}

		if (typeof target[name] == "object") {
			var p = new Proxy(target[name], Debug.ProxyHandler);
			target[name].__objPath = target.__objPath + (Array.isArray(target) ? "["+ name +"]"  : "['" + name + "']");

			target[name] = p;
		} else if (typeof target[name] == "function") {
			return function(...args) {
				try {
					return target[name].apply(this.__isProxy ? this.__wrappedObject : this, args);
				} catch(e) {
					console.log(this.__isProxy);
					console.log("Failed to call", name, "because", e);
					console.log("Stack", e.stack);
				}
			};
		}

		return target[name];
	},

	set : function(target, prop, value) {
		/*
		Debug.send({
			action: "eval",
			data: target.__objPath + "['"+prop+"'] = " + (typeof value == "string" ? '"'+ value +'"' : value)
		})
		console.log("Set", value, "on", prop, target.__objPath);
		*/
		target[prop] = value;
	},
};

Debug.installProxyOnDocument = function(){
	document.__objPath = "RemoteDocument";
	var p = new Proxy(document, Debug.ProxyHandler);

	var set = new Set();
	set.add(p);

	var recurse = function(obj) {
		for (var i in obj) {
			try {
				if (obj[i] && typeof obj[i] == "object" && !set.has(obj[i])) {
					set.add(obj[i]);
					recurse(obj[i]);
					set.delete(obj[i]);
				}
			} catch(e) {

			}
		}
	};

	recurse(p);
};

Debug.getFlatObjects = function(where) {
	var tmpSet = new Set();
	tmpSet.add(where);
	var startTime = +new Date();

	function getElements(root) {
		var obj = Array.isArray(root) ? [] : {};

		for (var i in root) {
			//if (!root.hasOwnProperty(i)) continue;
			if (typeof root[i] == "function") {
				obj[i] = "[function]";
				continue;
			} else if (typeof root[i] == "object") {
				if (tmpSet.has(root[i])) {
					obj[i] = "[Circular reference]";
					continue;
				}
				tmpSet.add(root[i]);
				obj[i] = getElements(root[i]);
				tmpSet.delete(root[i]);
			} else {
				obj[i] = root[i];
			}
		}

		return obj;
	}

	var ret = getElements(where);

	console.log("[getFlatObject time]", +new Date() - startTime, "Properties");

	return ret;
};

/* -------------------------------------------------------------------------- */

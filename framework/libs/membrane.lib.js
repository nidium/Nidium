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

"use strict";

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


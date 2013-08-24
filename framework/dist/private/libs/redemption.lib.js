/* -------------------------------------------------------------------------- */
/* RedemptionJS, an implementation of C like, low footprint binary structures */
/* -------------------------------------------------------------------------- */
/* The following implementation provides portable, memory-safe, efficient,    */
/* and structured access to compact binary data.                              */
/* -------------------------------------------------------------------------- */
/* More @ http://wiki.ecmascript.org/doku.php?id=harmony:binary_data          */
/* -------------------------------------------------------------------------- */
/* Author : Vincent Fontaine                                                  */
/* -------------------------------------------------------------------------- */

"use strict";

/* -------------------------------------------------------------------------- */

Object.createFrozenEntity = function(scope, property, value){
	Object.defineProperty(scope, property, {
		value : value,
		enumerable : true,
		writable : false,
		configurable : false
	});
};

Object.defineReadOnlyProperties = function(element, props){
	for (var key in props){
		if (props.hasOwnProperty(key)){
			Object.createFrozenEntity(element, key, props[key]);
		}
	}
};

Object.defineReadOnlyProperties(Native.scope, {
	// unsigned integer types
	get uint8(){
		return {m:Uint8Array, l:1};
	},
	get uint16(){
		return {m:Uint16Array, l:2};
	},
	get uint32(){
		return {m:Uint32Array, l:4};
	},

	// signed integer types
	get int8(){
		return {m:Int8Array, l:1};
	},
	get int16(){
		return {m:Int16Array, l:2};
	},
	get int32(){
		return {m:Int32Array, l:4};
	},

	// floating point types
	get float32(){
		return {m:Float32Array, l:4};
	},
	get float64(){
		return {m:Float64Array, l:8};
	}
});

var StructType = function(s){
	var self = this,
		seek = 0,
		index = 0,
		shaders = [];

	this.bytes = 0;
	this.buffer = null;
	this.__views = [];

	for (var i in s){
		var size = 1,
			t = s[i];

		if (t && t.m && s.hasOwnProperty(i)) {
			shaders.push({
				index : index++,
				prop : i,
				type : t.m,
				seek : seek,
				size : size
			});

			seek += t.l * size;
		}
	}

	var constructor = function(...n){
		var that = this;
		if (n.length > shaders.length){
			throw "Mismatch number of parameters. "+shaders.length+" expected.";
		}
		n.map(function(v, index){
			var prop = shaders[index].prop;
			that[prop] = v;
		});
	};
	
	constructor.prototype = this;

	var defineProperty = function(prop, index){
		var accessor = constructor.prototype.__views[index];
		
		Object.defineProperty(constructor.prototype, prop, {
			configurable : false,
			enumerable : true,

			set : function(value){
				accessor[0] = value;
			},

			get : function(){
				return accessor[0];
			}
		});
	};

	if (seek!=0) {
		this.bytes = seek;
		this.buffer = new ArrayBuffer(this.bytes);

		for (var i=0; i<shaders.length; i++){
			var prop = shaders[i].prop,
				f = shaders[i].type,
				view = new f(this.buffer, shaders[i].seek, shaders[i].size);

			constructor.prototype.__views[i] = view;
			defineProperty(prop, i);
		}
	}

	return constructor;
};




/* ---------- */
/* USAGE DEMO */ 
/* ---------- */

const Point2D = new StructType({
	x: uint32,
	y: uint32
});

const Color = new StructType({
	r: uint8,
	g: uint8,
	b: uint8
});

var p = new Point2D(7, 8);

console.log(p.x, p.y);


var z = [7, 8];

var ClassicPoint = function(x, y){
	this.x = x;
	this.y = y;
};

var q = new ClassicPoint(7, 8);

var u = +new Date();
for (var i=0; i<150000; i++){
	p.x += 1;
	p.y += 1;
}
console.log( (+new Date())-u  );



var u = +new Date();
for (var i=0; i<150000; i++){
	q.x += 1;
	q.y += 1;
}
console.log( (+new Date())-u  );


var u = +new Date();
for (var i=0; i<150000; i++){
	p.__views[0] += 1;
	p.__views[1] += 1;
}
console.log( (+new Date())-u  );


var u = +new Date();
for (var i=0; i<150000; i++){
	z[0] += 1;
	z[1] += 1;
}
console.log( (+new Date())-u  );




/*

-- TODO : --------------------

const Pixel = new StructType({
	point: Point2D,
	color: Color
});


const Triangle = new ArrayType(Pixel, 3);
 
let t = new Triangle([
	{ point: { x:  0, y: 0 }, color: { r: 255, g: 255, b: 255 } },
	{ point: { x:  5, y: 5 }, color: { r: 128, g: 0,   b: 0   } },
	{ point: { x: 10, y: 0 }, color: { r: 0,   g: 0,   b: 128 } }
]);

*/
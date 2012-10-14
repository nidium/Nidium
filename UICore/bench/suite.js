/* -------------------------- */
/* Native (@) 2012 Stight.com */
/* -------------------------- */

/*
BenchThis("Read/Write CStruct Point", 50000, function(i){
});

BenchThis("Read/Write Object Litteral Point", 50000, function(i){
});
*/




var receiver
var argument
var o = {
  a: 6,
  b: {bb: 8},
  f: function(x) { receiver = this; argument = x; return x },
  g: function(x) { receiver = this; argument = x; return x.a },
  h: function(x) { receiver = this; argument = x; this.q = x },
  s: function(x) { receiver = this; argument = x; this.x = {y: x}; return this }
}
o[2] = {c: 7}
var m = Membrane(o)
var w = m.wrapper


o.a = 8;

echo(o.a);


var f = w.f
var x = f(66)
var x = f({a: 1})
var x = w.f({a: 1})
var a = x.a

echo(4, (new w.h(4)).q)

m.gate.revoke();

w.a;

var g = function(){ echo(w.a) };

g();
/*
var m = Membrane(p);


var z = m.wrapper;

echo(z.g);

*/

/*

Object.getOwnPropertyNames(Object).forEach(function(n){
	//Object.defineProperty(r, n, Object.getOwnPropertyDescriptor(o, n));
	console.log(n);
});

*/



/*

canvas.fillStyle = "rgba(255, 0, 0, 1)";

BenchThis("1024x768 FillRect", 100, function(i){
	canvas.fillRect(0, 0, 1024, 768);
});

BenchThis("1024x768 FillRect", 200, function(i){
	canvas.fillRect(0, 0, 1024, 768);
});

BenchThis("1024x768 FillRect", 400, function(i){
	canvas.fillRect(0, 0, 1024, 768);
});

BenchThis("1024x768 FillRect", 800, function(i){
	canvas.fillRect(0, 0, 1024, 768);
});

BenchThis("1024x768 FillRect", 1600, function(i){
	canvas.fillRect(0, 0, 1024, 768);
});

BenchThis("1024x768 FillRect", 3200, function(i){
	canvas.fillRect(0, 0, 1024, 768);
});

BenchThis("1024x768 FillRect", 6400, function(i){
	canvas.fillRect(0, 0, 1024, 768);
});

BenchThis("1024x768 FillRect", 12800, function(i){
	canvas.fillRect(0, 0, 1024, 768);
});

*/
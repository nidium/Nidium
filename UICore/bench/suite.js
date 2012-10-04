/* -------------------------- */
/* Native (@) 2012 Stight.com */
/* -------------------------- */



BenchThis("SpiderMonkey Math.sin()", 3000000, function(i){
	Math.acos(i);
});


BenchThis("FastMath.sin()", 3000000, function(i){
	FastMath.acos(i);
});


BenchThis("SpiderMonkey Math.sin()", 300000, function(i){
	Math.sin(i);
});

BenchThis("Native.sin()", 300000, function(i){
	Native.sin(i);
});


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
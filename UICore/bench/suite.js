/* -------------------------- */
/* Native (@) 2012 Stight.com */
/* -------------------------- */

BenchThis("mul", 30000000, function(i){
	var d = Math.distance(20, 20, 500, 500);
});

BenchThis("pow", 30000000, function(i){
	var d = Math.FastDistance(20, 20, 500, 500);
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
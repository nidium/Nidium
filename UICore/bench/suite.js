/* -------------------------- */
/* Native (@) 2012 Stight.com */
/* -------------------------- */

BenchThis("Random FillStyle", 150000, function(i){
	canvas.fillStyle = "rgba("+Math.random()*255+", "+Math.random()*255+", "+Math.random()*255+", "+Math.random()*255+")";
});

BenchThis("FillRect", 500000, function(i){
	canvas.fillRect(0, 0, 1024, 768);
});


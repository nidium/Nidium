/* -------------------------- */
/* Native (@) 2012 Stight.com */
/* -------------------------- */

/* Native Sandboxed Thread Demo */

var t = new Thread(function(){
	for (var i=0; i<500000000; i++){
		if (i%10000000 == 0) {
			this.send(i);
		}
	}
});

t.onmessage = function(e){
	echo(e.message);
};

t.onfinish = function(r){
	echo("Thread returned", r);
}

canvas.requestAnimationFrame(function(){
	canvas.clearRect(0, 0, 1024, 768);
});
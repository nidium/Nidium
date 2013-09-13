/* ------------------------+------------- */
/* Native Framework 2.0    | Falcon Build */
/* ------------------------+------------- */
/* (c) 2013 nidium.com - Vincent Fontaine */
/* -------------------------------------- */

Native.showFPS(true);

var main = new Application({
	id : "main",
	background : "black"
});

createParticle = function(x, y){
	return new UIParticle(main, {
		radius : 32,
		left : x,
		top : y,
		shadowBlur : 30,
		background : "rgba(255, 0, 255, 0.3)",
		alpha : 0.1
	});
};

var p = [],
	nbparticles = 400;

for (var i=0; i<nbparticles; i++){
	var x = Math.round(1024*Math.random()),
		y = Math.round(768*Math.random());

	p[i] = createParticle(x, y);
	p[i].dx = 6*Math.random();
	p[i].dy = 6*Math.random();
	p[i].opacity = 0.1+Math.random();
}

setInterval(function(){
	var t = +new Date();
	for (var i=0; i<nbparticles; i++){
		var x = p[i]._left,
			y = p[i]._top;

		x += p[i].dx;
		y += p[i].dy;

		p[i].setCoordinates(x, y);

		if (x>1024 || x<0) {
			p[i].dx = -p[i].dx;
		}

		if (y>768 || y<0) {
			p[i].dy = -p[i].dy;
		}
	}
	//console.log((+new Date()) - t);

}, 16);


/*

particle.trajectory = [
	10, 10,
	150, 150,
	600, 250,
	10, 10
];

particle.addEventListener("drag", function(e){
	this.left += e.xrel;
	this.top += e.yrel;
});


particle.frameStep = 0.1;
particle.speed = 50;
particle.motion = Math.physics.elasticOut;
//particle.startAnimation();

particle.trajectory = function(time){
	return {
		left : 150 + 88*sin(time),
		top : 150 + 88*cos(time*2)
	}
};
*/
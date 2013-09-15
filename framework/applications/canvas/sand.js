var width = 1024,
	height = 768,
	ctx = window.canvas.getContext("2d"),
	screen = ctx.createImageData(width, height);

var max_particles = 140000,
	particles = [],
	rnd = Math.random,
	max = Math.max,
	i = 0;

while (particles.length<max_particles) {
	particles.push({
		x : ( rnd() * width ) | 0,
		y : ( rnd() * height ) | 0,
		vx : 0,
		vy : 0,
		alpha : 128+Math.round(64*Math.random())
	});
}

function erase(i) {
	var p = particles[i];
	
	if ( p.x >= 0 && p.x < width && p.y >= 0 && p.y < height ){
		let idx = ( ( p.x | 0) + ( p.y | 0) * width ) * 4;
		screen.data[idx+3] = 0;
	}
}

function update(i){
	var p = particles[i];

	p.x += p.vx;
	p.y += p.vy;

	p.x = p.x < 0 ? width + p.x : p.x >= width ? p.x - width : p.x;
	p.y = p.y < 0 ? height + p.y : p.y >= height ? p.y - height : p.y;

	p.vy = 2*Math.sin(p.x/150) - 0.5;
	p.vx = 2*Math.sin(p.y/150) - 0.5;
}

function draw(i){
	var p = particles[i],
		d = ( (p.x|0) + (p.y|0) * width ) * 4;

	if ( p.x >= 0 && p.x < width && p.y >= 0 && p.y < height ) {
		screen.data[d+0] = 0;
		screen.data[d+1] = 0;
		screen.data[d+2] = 0;
		screen.data[d+3] = p.alpha;
	}
}

ctx.fillStyle = "#000000";
ctx.globalAlpha = 0.8;
ctx.requestAnimationFrame(function(){

	for (i=0; i<max_particles; i++){
		erase(i);
		update(i);
		draw(i);
	}
	ctx.putImageData(screen, 0, 0);
	ctx.fillRect(0, 0, width, height);
});

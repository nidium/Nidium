/*
 * ported from Paul Brunt's JSIK demo
 * http://js1k.com/2010-first/demo/753
 */

var text = "N A T i V E",
	image, 	point, a = b = c = i = 0,

	cw = 1024,
	ch = 900,

	v = canvas.createRadialGradient(cw/2, ch*2/3, 0, cw/2, ch+ch/8, ch/1.5),

	M = Math,

	SPHERE3D = [],
	TEXT3D = [],
	w = [],
	
	nb_particles = 4000,
	time = 16000,

	r = M.random,
	pi2 = M.PI * 2,

	zoom = 4,
	dx = 0,
	dy = 0,

	g, t, sin, cos,
	tx, ty;


canvas.fontSize = 13;
canvas.fillText(text, 0, 20);

tx = M.round(canvas.measureText(text)),
ty = 25;

image = canvas.getImageData(0, 0, tx, ty);
v.addColorStop(0, "#343");
v.addColorStop(1, "#000");

while (i < nb_particles) {
	a = r() * 2 - 1;
	g = M.pow(a*a + b*b + c*c, .5) / 2;
	
	SPHERE3D[i] = [
		a / g,
		b / g + 1,
		c / g
	];

	c = b;
	b = a;

	x = r() * tx;
	y = r() * ty;

	var xx = (x - tx * 0.5) / ty * 1.5,
		yy = (.5 - y / ty) * 1.5,
		zz = r() * 0.4;

	image.data[ ((y | 0) * tx + (x | 0)) * 4 + 3 ] > 9 ? TEXT3D[i++] = [xx, yy, zz] : 0;
}

function n(i) {
	return (w[i] - point[i]) * m*m*m*m*m + point[i];
}

canvas.requestAnimationFrame(function(){
	g = new Date % time / time * 8;
	t = g * pi2;
	sin = M.cos(t);
	cos = M.sin(t);

	canvas.fillStyle = v;
	canvas.fillRect(0, 0, cw, ch);

	for (i=0; i<nb_particles; i++) {
		point = TEXT3D[i];
		w = SPHERE3D[i];
		m = g >= 3 ? 0 : g >= 2 ? 1 - g % 1 : g >= 1 ? 1 : g % 1;

		d = n(2);
		h = n(1);
		l = n(0);

		c = 1 / (cos*l - sin*d - zoom) * ch;

		a = [
			(cw + c*(sin*l + cos*d)) * 0.5,
			(ch + (i%2 ? c*(-h-2)*0.6 : c*h)) * 0.5
		];
		canvas.fillStyle = i % 2 ? "#373" : "rgba("+M.round(c)+", "+M.round(255-d*0.5)+", "+M.round(l)+", 0.8)";
		canvas.fillRect(a[0] | 0, a[1] | 0, 0.6, 0.6);
	}
	canvas.globalAlpha = 0.4;
});

canvas.onmousewheel = function(e){
	zoom = zoom * (1+e.yrel*0.01);
};

canvas.onmousemove = function(e){
	dx = e.x - cw/2;
	dy = e.y - ch/2;
};






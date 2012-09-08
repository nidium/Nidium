/*
 * port from http://js1k.com/2010-first/demo/114
 */

var cw = 1024,
	ch = 768,
	c = cw > ch ? cw / 15 : ch / 15,
	d = canvas,
	e, f, g = [],
	h, i;

for (e = 0; e < 150; e++) {
	h = {
		x : Math.random() * cw,
		y : Math.random() * ch,
		a : Math.random() * 1E3,
	};
	
	h.b = h.a;

	g[e] = h;
}
var j, k, l,
	dd = 180 * Math.PI;


d.lineWidth = 2;
d.lineCap = "round";
var m;

canvas.requestAnimationFrame(function(){
	d.fillStyle = "rgba(0, 0, 0, 0.4)";
	d.fillRect(0, 0, cw, ch);

	for (e = 0; e < 150; e++) {
		h = g[e];
		h.a += Math.random() > 0.5 ? -1 : 1;
		h.b -= (h.b - h.a) * 0.01;
		m = h.b * 25 / dd;
		h.x += 1.5 * Math.cos(m);
		h.y += 1.5 * Math.sin(m);
		
		if (h.x < 0) h.x = cw;
		if (h.x > cw) h.x = 0;
		if (h.y < 0) h.y = ch;
		if (h.y > ch) h.y = 0;
		
		for (f = e + 1; f < 150; f++) {
			i = g[f];
			k = h.x - i.x;
			l = h.y - i.y;
			j = Math.sqrt(k*k + l*l);

			if (j < c) {
				d.strokeStyle = "rgba( 255, 100, 255, " + (c - j) / c + " )";
				d.beginPath();
				d.moveTo(h.x, h.y);
				d.lineTo(i.x, i.y);
				d.stroke()
			}

		}

		d.fillStyle = "#ffffff";
		d.fillRect(h.x - 1, h.y - 1, 3, 3)
	}
});
/* 
 * RealTime JavaScript Raytracer DEMO
 * Forked from Jonas Wagner's work
 * http://29a.ch/sandbox/2010/water/
 */


w = 63;
b = 255;
G = canvas;
M = Math;
Q = M.max;
V = 12;
v = -V;
c = [];
i = 64;
f = [i];
s = 0;
while (i--){
	c[i] = i * 4 + ",0, 0, "+i/64+")";
	c[i + 63] = b + "," + i * 4 + ",0, "+0.8+")";
	c[i + 127] = b + ",255," + i * 4 + ", "+1+")";
	f[i] = []
}
function L(l, m, n, o) {
	k = (o - m) / (n - l);
	m = m - l * k;
	for (i = M.min(l, n); i < Q(l, n); i += 1 / Q(1, M.abs(k))) f[i & b][(k * i + m) & b] = b
}
function D(l, m, n, o, p, q) {
	P = [l, o];
	R = [m, p];
	S = [n, q];
	X = [];
	Y = [];
	for (i = 0; i < 2;) {
		Z = (-P[i] * O * O - R[i] * H * O + S[i] * H) + w - 5;
		X[i] = w * ((-P[i] * H + R[i] * O) / Z) + (w / 2);
		Y[i] = w * ((-P[i] * O * H - R[i] * H * H - S[i++] * O) / Z) + (w / 2) + 8
	}
	L(X[0], Y[0], X[1], Y[1])
}

canvas.scale(1.5, 1.5);

var zoom = 180;

canvas.onmousewheel = function(e){
	zoom = Math.min(500, Math.max(140, zoom * (1+e.yrel*0.01)));
};

canvas.requestAnimationFrame(function(){
	H = M.sin(s += .06);
	O = M.cos(s);

	canvas.fillStyle = "#000000";
	canvas.fillRect(0, 0, 1024, 768);

	for (y = 0; y < 88; y++) for (x = 0; x < w; x++) {
		f[x][y] = ((f[(x-1) & w][y+1] + f[x][y+1] + f[x+1][y+1] + f[x][(y+2) & w]) << 5) / zoom;
		G.fillStyle = "rgba(" + c[ Math.min(f[x][y] & b, 190) ];
		G.fillRect(150+x*6, 0+y*6, 5, 5)
	}

	for (y = 0; y < 72;) eval("D(" + "vvvVvvVvvVVvVVvvVvvVvvvvvvVVvVVvVVVVVVVvVVvVVvvVvvVvvvVvVVvvVVVVVvvVVvVv".substring(y, y += 6).split("") + ")")
});
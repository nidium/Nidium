/*
 * Port of Markus Persson's demo (http://jsfiddle.net/uzMPU/)
 */

var ctx = window.canvas.getContext("2d");
var pixels;

var w = 256;
var h = 192;

ctx.scale(4, 4);

var k = 16,
	kk = 32;

/*
var map = new Array(64 * 64 * 64);
var texmap = new Array(k * k * 3 * k);
*/

var map = new Uint8Array(64 * 64 * 64);
var texmap = new Uint32Array(k * k * 3 * k);


function init() {
	for ( var i = 1; i < k; i++) {
		var br = 255 - ((Math.random() * 96) | 0);
		for ( var y = 0; y < k * 3; y++) {
			for ( var x = 0; x < k; x++) {
				var color = 0x966C4A;

				if (i == 4) color = 0x7F7F7F;

				if (i != 4 || ((Math.random() * 3) | 0) == 0) {
					br = 255 - ((Math.random() * 96) | 0);
				}

				if ((i == 1 && y < (((x * x * 3 + x * 81) >> 2) & 3) + 18)) {
					color = 0x6AAA40;
				} else if ((i == 1 && y < (((x * x * 3 + x * 81) >> 2) & 3) + 19)) {
					br = br * 2 / 3;
				}

				if (i == 7) {
					color = 0x675231;
					if (x > 0 && x < 15
						&& ((y > 0 && y < 15) || (y > 32 && y < 47))) {
						color = 0xBC9862;
						var xd = (x - 7);
						var yd = ((y & 15) - 7);
					
						if (xd < 0) xd = 1 - xd;
						if (yd < 0) yd = 1 - yd;
						if (yd > xd) xd = yd;

						br = 196 - ((Math.random() * 32) | 0) + xd % 3 * 32;
					} else if (((Math.random() * 2) | 0) == 0) {
						br = br * (150 - (x & 1) * 100) / 100;
					}
				}

				if (i == 5) {
					color = 0xB53A15;
					if ((x + (y >> 2) * 4) % 8 == 0 || y % 4 == 0) {
						color = 0xBCAFA5;
					}
				}

				if (i == 9) {
					color = 0x4040ff;
				}

                var brr = br;

				if (y >= 32) brr /= 2;

				if (i == 8) {
					color = 0x50D937;
					if (((Math.random() * 2) | 0) == 0) {
						color = 0;
						brr = 255;
					}
				}

				var col = (((color >> k) & 0xff) * brr / 255) << k
						| (((color >> 8) & 0xff) * brr / 255) << 8
						| (((color) & 0xff) * brr / 255);
				texmap[x + y * k + i * 256 * 3] = col;
			}
		}
	}

	for ( var x = 0; x < 64; x++) {
		for ( var y = 0; y < 64; y++) {
			for ( var z = 0; z < 64; z++) {
				var i = z << 12 | y << 6 | x;

				/*
				var yd = (y - 64) * 0.02;
				var zd = (z - 64) * 0.02;
				*/

				var yd = (y - 32) * 0.2;
				var zd = (z - 32) * 0.2;

				map[i] = (Math.random() * k) | 0;

				if (Math.random() > Math.sqrt(Math.sqrt(yd * yd + zd * zd)) - 0.8)
					map[i] = 0;
			}
		}
	}

	pixels = ctx.createImageData(w, h);

	for ( var i = 0; i < w * h; i++) {
		pixels.data[i * 4 + 3] = 255;
	}
};

function clock() {
	renderMinecraft();
	ctx.putImageData(pixels, 0, 0);
};

var f = 0;
function renderMinecraft() {
	var xRot = Math.sin(Date.now() % 10000 / 10000 * Math.PI * 2) * 0.4
	        + Math.PI / 2;
	var yRot = Math.cos(Date.now() % 10000 / 10000 * Math.PI * 2) * 0.6;
	var yCos = Math.cos(yRot);
	var ySin = Math.sin(yRot);
	var xCos = Math.cos(xRot);
	var xSin = Math.sin(xRot);

	var ox = 32.5 + Date.now() % 10000 / 10000 * 256;
	var oy = 32.5;
	var oz = 32.5;

	f++;
	for ( var x = 0; x < w; x++) {
		var ___xd = (x - w / 2) / h;
		for ( var y = 0; y < h; y++) {
			var __yd = (y - h / 2) / h;
			var __zd = 1;

			var ___zd = __zd * yCos + __yd * ySin;
			var _yd = __yd * yCos - __zd * ySin;

			var _xd = ___xd * xCos + ___zd * xSin;
			var _zd = ___zd * xCos - ___xd * xSin;

			var col = 0;
			var br = 255;
			var ddist = 0;

			var closest = 32;
			for ( var d = 0; d < 3; d++) {
				var dimLength = _xd;
				if (d == 1) dimLength = _yd;
                if (d == 2) dimLength = _zd;

				var ll = 1 / (dimLength < 0 ? -dimLength : dimLength);
				var xd = (_xd) * ll;
				var yd = (_yd) * ll;
				var zd = (_zd) * ll;

				var initial = ox - (ox | 0);
				if (d == 1) initial = oy - (oy | 0);
				if (d == 2) initial = oz - (oz | 0);
				if (dimLength > 0) initial = 1 - initial;

				var dist = ll * initial;

				var xp = ox + xd * initial;
				var yp = oy + yd * initial;
				var zp = oz + zd * initial;

				if (dimLength < 0) {
					if (d == 0) xp--;
					if (d == 1) yp--;
					if (d == 2) zp--;
				}

               
				while (dist < closest) {
					var tex = map[(zp & 63) << 12 | (yp & 63) << 6 | (xp & 63)];

					if (tex > 0) {
						var u = ((xp + zp) * k) & 15;
						var v = ((yp * k) & 15) + k;
					
						if (d == 1) {
							u = (xp * k) & 15;
							v = ((zp * k) & 15);
							if (yd < 0) v += 32;
						}

						var cc = texmap[u + v * k + tex * 256 * 3];
					
						if (cc > 0) {
							col = cc;
							ddist = 255 - ((dist / 32 * 255) | 0);
							br = 255 * (255 - ((d + 2) % 3) * 50) / 255;
							closest = dist;
						}
					}

					xp += xd;
					yp += yd;
					zp += zd;
					dist += ll;
				}

			}

			var r = ((col >> k) & 0xff) * br * ddist / (255 * 255);
			var g = ((col >> 8) & 0xff) * br * ddist / (255 * 255);
			var b = ((col) & 0xff) * br * ddist / (255 * 255);// + (255 -

			pixels.data[(x + y * w) * 4 + 0] = r;
			pixels.data[(x + y * w) * 4 + 1] = g;
			pixels.data[(x + y * w) * 4 + 2] = b;

		}
	}
}

init();

ctx.requestAnimationFrame(function(){
	clock();    	
});




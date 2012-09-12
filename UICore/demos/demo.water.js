/* 
 * RealTime JavaScript Raytracer DEMO
 * Forked from Jonas Wagner's work
 * http://29a.ch/sandbox/2010/water/
 */

var floor = Math.floor,
	ctx = canvas,
	cw = 320,
	ch = 240,
	width = cw,
	height = ch,
	size = width * height,
	buffer0 = [],
	buffer1 = [],
	aux, i, texture;


function clamp(x, min, max) {
	if (x < min) return min;
	if (x > max) return max - 1;
	return x;
}

function getDataFromImage(img) {
	ctx.drawImage(img, 0, 0, img.width, img.height);
	return ctx.getImageData(0, 0, img.width, img.height);
}

function loadImage(src, callback) {
	var img = new Image();
	img.src = src;
	callback(img);
}

function disturb(x, y, z) {
	if (x < 2 || x > width - 2 || y < 1 || y > height - 2) return;
	var i = x + y * width;
	buffer0[i] += z;
	buffer0[i - 1] -= z;
}

function process() {
	var img = ctx.getImageData(0, 0, width, height),
		data = img.data,
		i, x;

	// average cells to make the surface more even
	for (i = width + 1; i < size - width - 1; i += 2) {
		for (x = 1; x < width - 1; x++, i++) {
			buffer0[i] = (buffer0[i] + buffer0[i + 1] + buffer0[i - 1] + buffer0[i - width] + buffer0[i + width]) / 5;
		}
	}
	for (i = width + 1; i < size - width - 1; i += 2) {
		for (x = 1; x < width - 1; x++, i++) {

			// wave propagation
			var waveHeight = (buffer0[i - 1] + buffer0[i + 1] + buffer0[i + width] + buffer0[i - width]) / 2 - buffer1[i];
			buffer1[i] = waveHeight;
			
			// calculate index in the texture with some fake referaction
			var ti = i + floor((buffer1[i - 2] - waveHeight) * 0.08) + floor((buffer1[i - width] - waveHeight) * 0.08) * width;
			
			// clamping
			ti = ti < 0 ? 0 : ti > size ? size : ti;
			
			// some very fake lighting and caustics based on the wave height
			// and angle
			var light = waveHeight * 2.0 - buffer1[i - 2] * 0.6,
				i4 = i * 4,
				ti4 = ti * 4;

			// clamping
			light = light < -10 ? -10 : light > 100 ? 100 : light;
			data[i4] = texture.data[ti4] + light;
			data[i4 + 1] = texture.data[ti4 + 1] + light;
			data[i4 + 2] = texture.data[ti4 + 2] + light;
		}
	}

	// rain
	disturb(floor(Math.random() * width), floor(Math.random() * height), Math.random() * 1000);
	aux = buffer0;
	buffer0 = buffer1;
	buffer1 = aux;

	canvas.fillRect(0, 0, 1024, 768);
	ctx.putImageData(img, 0, 0);
}

for (i = 0; i < size; i++) {
	buffer0.push(0);
	buffer1.push(0);
}

loadImage("demos/demo.water320x240.jpg", function(img){

	texture = getDataFromImage(img);
	ctx.fillRect(0, 0, width, height);

	canvas.requestAnimationFrame(function(){
		process();
	});

});

canvas.onmousemove = function(e) {
	disturb(floor(e.x / cw * width), floor(e.y / ch * height), 15000);
}



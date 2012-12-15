// https://bugzilla.mozilla.org/show_bug.cgi?id=790628

var t0 = new Date();

var	width = 320,
	height = 240,

	size = width * height,
	data = new Uint8ClampedArray(size*4),

	texture = new Uint8ClampedArray(160*120*4);
	//texture = new Uint8ClampedArray(320*240*4); // no bug with 320*240

function process() {
	for (var i = width; i < size - width; i += 2) {
		data[i * 4] = texture[i * 4];
	}
}

for (var i = 0; i < 30; i++) process();


echo(new Date() - t0);



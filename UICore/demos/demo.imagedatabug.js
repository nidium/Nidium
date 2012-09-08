function init() {
	var img = new Image(),
		z, data;

	img.onload = function(){};
	img.src = 'demo.realtimeRayTracer.jpeg';

	img.width = 577; // KO, img.width devrait contenir 1536 au lieu de undefined
	img.height = 768;  // KO, img.height devrait contenir 2048 au lieu de undefined

	z = getDataFromImage(img);
	data = z.data;

	echo(z.width); // OK = 1536
	echo(z.height); // OK = 2048
	echo(z.data.length); // OK = 12582912 = 1536*2048*4


	// BUG : impossible de mettre un echo dans une boucle de plus de 4000 iterations sinon plantage

	/*
	for (var i=0; i<1000; i+=4){
		echo(data[i], data[i+1], data[i+2], data[i+3]);
	}
	*/

	// OK avec la taille altérée de l'image (577x768)
	// KO avec la taille originale de l'image (1536x2048)
	canvas.putImageData(z, 0, 0);

}

function getDataFromImage(img) {
	canvas.drawImage(img, 0, 0, img.width, img.height); // Ok, affiche bien l'image
	var zz = canvas.getImageData(0, 0, img.width, img.height);
	canvas.fillStyle = "#ffffff"; // OK
	canvas.fillRect(0, 0, img.width, img.height); // OK
	return zz;
}

init();
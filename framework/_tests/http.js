/* -------------------------- */
/* Native (@) 2012 Stight.com */
/* -------------------------- */

/*
var h = new Http("http://code.downthemall.net/maierman/metalink-test/test.png");
h.request(function(e){
	if (e.type == "image") {
		canvas.drawImage(e.data, 0, 0);
	};
	twitwitwit();
});

h.oncomplete = function(){

};

h.onerror = function(e){

};

function twitwitwit(){
	var r = new Http("http://search.twitter.com/search.json?q=javascript&rpp=500&include_entities=true&result_type=mixed");

	r.request(function(ev){
		if (ev.type == "json") {
			for (let i = 0; i < ev.data.results.length; i++){
				var p = new Http(ev.data.results[i].profile_image_url).request(function(e){
					var image = e.data,
						px = 1024 - image.width,
						py = 768 - image.height;

					canvas.drawImage(image, Math.random()*px, Math.random()*py);
				});
			}
		}
	});
}

*/





var ilist = [];


var h = new Http("http://search.twitter.com/search.json?q=javascript&rpp=200&include_entities=true&result_type=mixed");
h.request(function(ev) {

	if (ev.type == "image")
		canvas.drawImage(ev.data, 0, 0);
	else if (ev.type == "json") {
		for (let i = 0; i < ev.data.results.length; i++) {
			var p = new Http(ev.data.results[i].profile_image_url);
			p.request(function(e) {
				if (e.type == "image") {
					canvas.drawImage(e.data, Math.random()*300, Math.random()*300);
					ilist.push(e.data);
				}
				else
					echo("Not an Image", e.type, e.data);
			});
		}

	}
		
})

canvas.onmousemove = function(ev) {
	//canvas.translate(ev.xrel, ev.yrel);
}

canvas.fillStyle = "#FFF";

canvas.requestAnimationFrame(function() {
	canvas.fillRect(0, 0, canvas.width, canvas.height);

	for (var i = 0; i < ilist.length; i++) {
		canvas.drawImage(ilist[i], Math.random()*600, Math.random()*600);
	}
});









/*
var file = new DataStream("http://www.google.fr/logo.gif");
file.buffer(4096);
file.chunk(2048);
file.seek(154555);
file.ondata = function(data){
	for (var i=0; i<data.length; i++){
		echo(data[i]);
	}
};
file.open();
file.oncomplete = function(data){};
file.onclose = function(){};
*/

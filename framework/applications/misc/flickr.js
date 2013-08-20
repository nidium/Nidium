/* -------------------------- */
/* Native (@) 2012 Stight.com */
/* -------------------------- */

Native.showFPS(true);

var tag = "boobs";

var ilist = new Set();

var h = new Http("http://api.flickr.com/services/rest/?method=flickr.photos.search&api_key=94772f188a7f918c9d3116cae17d43b8&tags="+encodeURIComponent(tag)+"&format=json&nojsoncallback=1&per_page=500");
h.request(function(ev) {

if (ev.type == "json") {
	console.log("loading images...", ev.data.photos.photo.length);
	for (let i = 0; i < ev.data.photos.photo.length; i++) {
		let item = ev.data.photos.photo[i];
		let iurl = 'http://farm' + item.farm + '.staticflickr.com/' + item.server + '/' + item.id + '_' + item.secret + '_n.jpg';
		var img = new Image();
		img.src = iurl;
		img.onload = function() {
			canvas.drawImage(this, Math.random()*300, Math.random()*300);
			ilist.add(this);		
		}
	}
} else {
	console.log("failed");
}
		
})

canvas.onmousemove = function(ev) {
	//canvas.translate(ev.xrel, ev.yrel);
}

canvas.fillStyle = "#FFF";



canvas.requestAnimationFrame(function() {
	canvas.globalAlpha = 0.1;
	canvas.fillRect(0, 0, canvas.width, canvas.height);
	canvas.globalAlpha = 1;

	for (let i of ilist) {
		canvas.drawImage(i, Math.random()*1024, Math.random()*768);
	}
});

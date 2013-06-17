/* ------------------------+------------- */
/* Native Framework 2.0    | Falcon Build */
/* ------------------------+------------- */
/* (c) 2013 Stight.com - Vincent Fontaine */
/* -------------------------------------- */

var main = new Application({
	backgroundImage : "falcon/assets/back.png"
});

var	button = new UIButton(main, {
	label : "Do It"
}).center();

button.addEventListener("mouseup", function(e){
	var keyword = "flower",
		url = "http://api.flickr.com/services/rest/?method=flickr.photos.search&api_key=94772f188a7f918c9d3116cae17d43b8&tags="+encodeURIComponent(keyword)+"&format=json&nojsoncallback=1&per_page=5";

	var h = new HttpRequest('GET', url, null, function(e){
		if (e.type == "json"){
			processFlickr(e.data.photos.photo);
		} else {
			echo("failed");
		}
	});

});

/* ------------------------------------------------------------------------- */

var NSImage = function(parent, url){
	var img = new Image();
	img.onload = function(){
		pic.setBackgroundImage(img);
		pic.width = img.width;
		pic.height = img.height;
		pic.visible = true;
	};

	var	pic = new UIView(parent, {
		visible : false,
		left : Math.random()*900,
		top : Math.random()*600,
		width : 1,
		height : 1,
		background : "#000000",
		radius : 6,
		shadowBlur : 6,
		shadowColor : "black"
	});

	img.src = url;
	return pic;
};

var processFlickr = function(pictures){
	for (var i=0; i<pictures.length; i++) {
		var item = pictures[i],
			iurl = 'http://farm' + item.farm + '.staticflickr.com/' + item.server + '/' + item.id + '_' + item.secret + '_n.jpg';

		var	pic = new NSImage(main, iurl);
		pic.addEventListener("mousedown", function(e){
			this.bringToFront();
			e.stopPropagation();
		}, false);

		pic.addEventListener("drag", function(e){
			this.left += e.xrel;
			this.top += e.yrel;
		}, false);
	}
};


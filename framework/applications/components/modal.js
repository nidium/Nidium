/* ------------------------+------------- */
/* Native Framework 2.0    | Falcon Build */
/* ------------------------+------------- */
/* (c) 2013 Stight.com - Vincent Fontaine */
/* -------------------------------------- */

var main = new Application({
	id : "main",
	backgroundImage : "private/assets/back.png"
});

var modal = new UIModal(main);

var	button = new UIButton(main, {
	id : "button",
	left : 966,
	top : 8,
	label : "Do It"
});

button.addEventListener("mouseup", function(e){
	modal.open();

	var h = new HttpRequest('GET', url, null, function(e){
		if (e.type == "json"){
			processFlickr(e.data.photos.photo);
		} else {
			console.log("failed");
		}
	});

});


var resize = new UIButton(modal.contentView).move(10, 10);
resize.setProperties({
	label : "<  >",
	background : "black",
	position : "fixed"
});

resize.addEventListener("mouseup", function(e){
	if (modal.toggle) {
		modal.toggle = false;
		modal.spinner.show();
		modal.spinner.opacity = 0.5;
		modal.spinner.fadeOut(500);

		modal.contentView.animate(
			"width", modal.contentView.width, 320,
			500, null,
			Math.physics.elasticOut
		);

		modal.contentView.animate(
			"height", modal.contentView.height, 200,
			400, null,
			Math.physics.quadIn
		);

	} else {
		modal.toggle = true;
		modal.spinner.show();
		modal.spinner.opacity = 0.5;
		modal.spinner.fadeOut(500);

		modal.contentView.animate(
			"width", modal.contentView.width, 800,
			600, null,
			Math.physics.elasticOut
		);

		modal.contentView.animate(
			"height", modal.contentView.height, 600,
			500, null,
			Math.physics.quadOut
		);
	}
});

var	bigview = new UIView(main, {
	id : "bigview",
	width : 1000,
	height : 700,
	background : "rgba(240, 80, 180, 0.2)",
	radius : 10
}).center().move(0, 20);


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
		left : Math.random()*600,
		top : Math.random()*400,
		width : 400,
		height : 400,
		background : "#000000",
		radius : 6,
		shadowBlur : 6,
		shadowColor : "black"
	});

	img.src = url;
	return pic;
};



var images = [],
	keyword = "audi",
	url = "http://api.flickr.com/services/rest/?method=flickr.photos.search&api_key=94772f188a7f918c9d3116cae17d43b8&tags="+encodeURIComponent(keyword)+"&format=json&nojsoncallback=1&per_page=10";


var processFlickr = function(pictures){
	for (var i=0; i<pictures.length; i++) {
		var item = pictures[i],
			iurl = 'http://farm' + item.farm + '.staticflickr.com/' + item.server + '/' + item.id + '_' + item.secret + '_n.jpg';

		var	pic = new NSImage(modal.contentView, iurl);
		pic.addEventListener("mousedown", function(e){
			this.bringToFront();
			e.stopPropagation();
		}, false);

		pic.addEventListener("drag", function(e){
			this.left += e.xrel;
			this.top += e.yrel;
		}, false);

		images.push(pic);
	}
};






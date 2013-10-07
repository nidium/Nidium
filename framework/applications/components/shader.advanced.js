/* -------------------------------------- */
/* High Level Shader API (advanced demo)  */
/* -------------------------------------- */
/* (c) 2013 nidium.com - Vincent Fontaine */
/* -------------------------------------- */

var main = new Application();

var	tabController = main.add("UITabController", {
	name : "helloTabs",
	background : "rgba(25, 26, 24, 0.9)"
});

document.status.open();


tabController.setTabs([
	/* Tab 0 */ {label : "main.js", class : "tab"},
	/* Tab 1 */ {label : "core.js", class : "tab"},
	/* Tab 2 */ {label : "hello.js", selected : true, class : "tab"},
	/* Tab 3 */ {label : "foo.cpp", class : "tab"},
	/* Tab 4 */ {label : "class.cpp", class : "tab"},
	/* Tab 5 */ {label : "opengl.cpp", class : "tab"},
	/* Tab 6 */ {label : "2.js", class : "tab"},
	/* Tab 7 */ {label : "rotation.js", class : "tab"},
	/* Tab 8 */ {label : "scale.js", class : "tab"},
	/* Tab 9 */ {label : "native.inc.js", class : "tab"}
]);

setTimeout(function(){
	var t1 = tabController.add("UITab");
	t1.background = "rgba(255, 50, 240, 0.90)";
	t1.shadowBlur = 0;
	t1.color = "black";
}, 1500);

var toolBar = main.add("UIElement", {
	top : 32,
	height : 30,
	background : "#e0e0e0",
	alpha : 0.25
});


var field = toolBar.add("UITextField", {
	left : 4,
	top : 4
});

field.value = "https://www.google.com/";
field.width = 418;




var	dropDownController = toolBar.add("UIDropDownController", {
	name : "helloDrop",
	left : 828,
	top : 4,
	radius : 2
});
dropDownController.setOptions([
	/* Tab 0 */ {label : "France", 			value : 5},
	/* Tab 1 */ {label : "Belgium", 		value : 7},
	/* Tab 2 */ {label : "Monaco", 			value : 9},
	/* Tab 3 */ {label : "United States",	value : 15, selected : true},
	/* Tab 4 */ {label : "Italy", 			value : 1},
	/* Tab 5 */ {label : "Spain", 			value : 3, class : "greenland"},
	/* Tab 6 */ {label : "Bulgaria",		value : 2, class : "greenland"},
	/* Tab 7 */ {label : "Romania", 		value : 4},
	/* Tab 8 */ {label : "Sweden", 			value : 6},
	/* Tab 8 */ {label : "China", 			value : 8},
	/* Tab 8 */ {label : "Korea", 			value : 10},
	/* Tab 8 */ {label : "Luxembourg", 		value : 11, disabled : true},
	/* Tab 8 */ {label : "Switzerland", 	value : 12},
	/* Tab 9 */ {label : "Japan", 			value : 13}
]);

var	button = new UIButton(toolBar, {
	id : "button",
	left : 974,
	top : 4,
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




var ShaderDemo = {
	init : function(){
		var self = this;

		document.shader("applications/components/shaders/flamme.s", function(p, u){
			self.start(p, u);
		});

		/*
		setTimeout(function(){
			main.layer.context.detachFragmentShader();
		}, 3000);
		*/

	},

	start : function(program, uniforms){
		var t = 0;

		this.createSlider(uniforms);

		setInterval(function(){
			uniforms.itime = t++;
		}, 16);
	},

	createSlider : function(uniforms){
		var self = this;

		this.slider1 = toolBar.add("UISliderController", {
			left : 500,
			top : 9,
			width : 90,
			background : '#161712',
			color : '#000000',
			progressBarColor : 'rgba(255, 255, 255, 1)',
			disabled : false,
			radius : 2,
			min : 0,
			max : 100,
			value : 0
		});

		this.slider2 = toolBar.add("UISliderController", {
			left : 610,
			top : 9,
			width : 90,
			background : '#161712',
			color : '#000000',
			progressBarColor : 'rgba(255, 255, 255, 1)',
			disabled : false,
			radius : 2,
			min : 0,
			max : 200,
			value : 0
		});

		this.slider3 = toolBar.add("UISliderController", {
			left : 720,
			top : 9,
			width : 90,
			background : '#161712',
			color : '#000000',
			progressBarColor : 'rgba(255, 255, 255, 1)',
			disabled : false,
			radius : 2,
			min : -100,
			max : 200,
			value : 0
		});

		this.slider1.addEventListener("change", function(e){
			uniforms.data = e.value;
		}, false);

		this.slider2.addEventListener("change", function(e){
			uniforms.param = e.value;
		}, false);

		this.slider3.addEventListener("change", function(e){
			uniforms.zoom = e.value;
		}, false);

		document.addEventListener("mousewheel", function(e){
			var gg = Math.round(uniforms.zoom + e.yrel*2);
			gg = Math.min(gg, 200);
			gg = Math.max(gg, -100);
			self.slider3.setValue(gg);
		});

	}

};

ShaderDemo.init();



var modal = new UIModal(main);

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

document.nss.add({
	".radio" : {
		left : 10,
		width : 200,
		name : "choice",
		fontFamily : "menlo",
		fontSize : 11,
		textShadowColor : "rgba(0, 0, 0, 0.05)",
		lineWidth : 1,
		background : "rgba(0, 0, 0, 0.08)",
		borderColor : "rgba(0, 0, 0, 0.20)",
		borderWidth : 1,
		radius : 10
	},

	".red" : {
		color : "red",
	},

	".green" : {
		color : "#e0ff60"
	},

	".white" : {
		color : "#e0e0e0"
	},

	".transparent" : {
		color : "black",
		background : "rgba(255, 255, 255, 0.2)",
		borderColor : null
	}
});


document.addEventListener("DOMContentLoaded", function(){

	var video = new UIVideo(main, {
		width : 640,
		height : 360
	}).center().load("media/bunny.mp4", function(e){
		this.player.play();
	});

	video.opacity = 0.8;

	video.player.onplay = function(){
		console.log("Start playing ...");
	}

	video.player.onpause = function(){};
	video.player.onstop = function(){};
	video.player.onerror = function(e){};

	video.shader("applications/components/shaders/flag.s", function(program, uniforms){
		var t = 0;
		setInterval(function(){
			uniforms.itime = t++;
		}, 16);
	});

/*
	var	radio1 = video.add("UIRadio", {
		top : 50,
		label : "Select this",
		value : "option1",
		selected : true,
		shadowBlur : 4,
		class : "radio"
	});

	var radio2 = video.add("UIRadio", {
		top : 78,
		value : "option2",
		label : "... or this",
		class : "radio red"
	});

	var radio3 = video.add("UIRadio", {
		top : 106,
		value : "option3",
		label : "... or this",
		class : "radio green"
	});

	var radio4 = video.add("UIRadio", {
		top : 134,
		value : "option4",
		label : "... or this",
		class : "radio white"
	});

	var radio5 = video.add("UIRadio", {
		top : 162,
		value : "option5",
		label : "... or this",
		class : "radio transparent"
	});

	var	c0 = video.add("UICheckBox", {
		left : 10,
		top : 190,
		label : "UICheckBox (radius = 0)",
		value : 5,
		selected : true
	});

	var	c1 = video.add("UICheckBox", {
		left : 10,
		top : 216,
		label : "... or this",
		value : 5,
		selected : true,
		background : "rgba(255, 255, 255, 0.4)",
		radius : 12
	});

	var LFO = video.add("UIDiagram", {
		left : 450, 
		top : 10,
		label : "Low Frequency Oscillator",
		background : "#222222",
		pins : [
		{label:"Pitch",			type:"output"},
		{label:"Frequency",		type:"output"},
		{label:"Resonnance",	type:"output"},
		{label:"Audio Out",		type:"output"}
		]
	});
	*/

});


/*

EfyxMe : second videoplayer crash

setTimeout(function(){

	var video2 = new UIVideo(main, {
		width : 640,
		height : 360
	}).center().load("media/bunny.mp4", function(e){
		this.player.play();
	});
	video2.opacity = 0.8;

}, 1500);

*/

/* ------------------------+------------- */
/* Native Framework 2.0    | Falcon Build */
/* ------------------------+------------- */
/* (c) 2013 Stight.com - Vincent Fontaine */
/* -------------------------------------- */

/* -------------------------------------------------------------------------- */
load("libs/misc.lib.js");
/* -------------------------------------------------------------------------- */

var main = new Application({
	id : "main",
	background : "white"
});

var	button = new UIButton(main, {
	id : "button",
	left : 966,
	top : 8,
	label : "Do It"
});

button.addEventListener("mousedown", function(e){

	view.scrollTop = 50;


/*

	view.animate(
		"scrollTop", 	// property
		view.scrollTop, 	// start value
		132, 		// end value
		850, 		// duration 850ms
		null,		// callback
		Math.physics.expoOut // motion equation
	);
*/
});

var	bigview = new UIView(main, {
	id : "bigview",
	left : 150,
	top : 58,
	width : 600,
	height : 400,
	background : "rgba(240, 80, 180, 0.5)",
	overflow : false,
	scrollbars : true,
	radius : 10
});

var	view = new UIView(bigview, {
	id : "view",
	left : 30,
	top : 30,
	width : 400,
	height : 250,
	background : "rgba(0, 0, 80, 0.5)",
	overflow : false,
	scrollbars : true
});

var	c0 = new UIView(view, {
	id : "c0",
	left : 128,
	top : 400,
	width : 40,
	height : 40,
	background : "#008800",
	fixed : true
});

var	c1 = new UIView(view, {
	id : "c1",
	left : 100,
	top : 50,
	width : 100,
	height : 150,
	background : "rgba(255, 255, 255, 0.7)",
	overflow : true,
	scrollbars : true
});

echo("contentHeight after c1", c1.contentHeight);

var	c2 = new UIView(c1, {
	id : "c2",
	left : 10,
	top : 10,
	width : 20,
	height : 20,
	background : "#0088DD"
});

echo("contentHeight after c2", c1.contentHeight);

var	c22 = new UIView(c1, {
	id : "c22",
	left : 50,
	top : 50,
	width : 20,
	height : 20,
	background : "#0088DD"
});

echo("contentHeight after c22", c1.contentHeight);

var	c3 = new UIButton(c1, {
	id : "c3",
	left : 30,
	top : 260,
	label : "button",
	background : "#0088DD"
});

echo("contentHeight after c3", c1.contentHeight);

var	c4 = new UIView(view, {
	id : "c4",
	left : 280,
	top : 10,
	width : 20,
	height : 20,
	background : "#ff0088"
});

echo("contentHeight after c4", c1.contentHeight);

document.addEventListener("load", function(){
//	echo("#view.contentHeight:", view.contentHeight);
//	view.scrollTop = 50;
});

Native.layout.getElementsByTagName("UIView").each(function(){
	this.addEventListener("drag", function(e){
		this.left += e.xrel;
		this.top += e.yrel;
	});
});


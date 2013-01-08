/* ------------------------+------------- */
/* Native Framework 2.0    | Falcon Build */
/* ------------------------+------------- */
/* (c) 2013 Stight.com - Vincent Fontaine */
/* -------------------------------------- */

__DEBUG_SHOW_LAYERS__ = true;

var main = new Application({background:"dark", id:"main"});

var	button = new UIButton(main, {
	left : 966,
	top : 8,
	label : "Do It"
});

button.addEventListener("mousedown", function(e){
	c0.animate(
		"width", 	// property
		c0.width, 	// start value
		350, 		// end value
		850, 		// duration 850ms
		null,		// callback
		Math.physics.expoOut // motion equation
	);
});


var	view = new UIView(main, {
	id : "blue 400x400",
	left : 200,
	top : 50,
	width : 400,
	height : 400,
	background : "rgba(0, 0, 80, 0.5)"
});

var	c0 = new UIView(view, {
	id : "green 40x40",
	left : 10,
	top : 10,
	width : 40,
	height : 40,
	background : "#008800"
});

/*
var	c1 = new UIView(view, {
	id : "red 100x100",
	left : 100,
	top : 100,
	width : 100,
	height : 100,
	background : "rgba(80, 0, 0, 0.5)"
});

var	c3 = new UIView(c1, {
	id : "inner blue 20x20",
	left : 10,
	top : 10,
	width : 20,
	height : 20,
	background : "#0088DD"
});

var	c4 = new UIView(view, {
	id : "rose 20x20",
	left : 280,
	top : 10,
	width : 20,
	height : 20,
	background : "#ff0088"
});
*/

Native.layout.getElementsByTagName("UIView").each(function(){
	this.addEventListener("drag", function(e){
		this.left += e.xrel;
		this.top += e.yrel;
	});
});

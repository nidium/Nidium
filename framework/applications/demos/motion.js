/* -------------------------- */
/* Native (@) 2012 Stight.com */
/* -------------------------- */

var main = new Application({background:"#262722"});

var	button = new UIButton(main, {
	left : 966,
	top : 8,
	label : "Do It"
});

var	view = new UIView(main, {
	left : 30,
	top : 200,
	width : 300,
	height : 300,
	background : "rgba(50, 50, 190, 0.8)",
	radius : 12
});

button.addEventListener("mousedown", function(e){
	view.animate(
		"left", 	// property
		view.left, 	// start value
		700, 		// end value
		850, 		// duration 850ms
		null,		// callback
		Math.physics.expoOut // motion equation
	);
});


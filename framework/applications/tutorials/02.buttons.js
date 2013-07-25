/* -------------------------- */
/* Native (@) 2013 Stight.com */
/* -------------------------- */

// attach a button to document
var	b1 = document.add("UIButton");
b1.label = "test";
b1.center();

// attach another button to document
var	b2 = document.add("UIButton", {
	top : 30,
	left : 100,
	label : "a red one",
	color : "white",
	background : "#ff0000"
});

// you can use the following syntax too
var	b3 = new UIButton(document, {
	top : 60,
	left : 200,
	height : 40,
	fontSize : 9,
	label : "OK, Cool",
	color : "rgba(255, 255, 255, 0.55)",
	background : "#8855ff",
	radius : 12
});


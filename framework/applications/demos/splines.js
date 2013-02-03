/* -------------------------- */
/* Native (@) 2013 Stight.com */
/* -------------------------- */

var main = new Application();

var spline = new UILine(main, {
	class : "spline",
	left : 100,
	top : 100,
	width : 300,
	height : 300,
	background : "rgba(0, 200, 0, 0.1)",
	color : "black",
	lineWidth : 5,
	displayControlPoints : true,
	vertices : [
		10, 30,
		120, 50,
		150, 150,
		10, 180
	]
});

spline.addEventListener("update", function(){
	setSplineBounds();
});

var setSplineBounds = function(){
	var b = spline.boundingRect;
	bounds.left = b.left;
	bounds.top = b.top;
	bounds.width = b.right - b.left;
	bounds.left = b.bottom - b.top;
};

var bounds = new UIElement(main, {
	opacity : 0.5,
	background : "red"
});

setSplineBounds();
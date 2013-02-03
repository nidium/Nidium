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
	lineWidth : 50,
	displayControlPoints : true,
	vertices : [
		10, 30,
		120, 50,
		150, 150,
		10, 180
	]
});

var setSplineBounds = function(){
	var b = spline.boundingRect,
		m = spline.lineWidth;

	bounds.left = b.left;
	bounds.top = b.top;
	bounds.width = b.width;
	bounds.height = b.height;
};

var bounds = new UIElement(spline, {
	opacity : 0.5,
	background : "red"
});
bounds.sendToBack();

setSplineBounds();

spline.addEventListener("update", function(){
	setSplineBounds();
});


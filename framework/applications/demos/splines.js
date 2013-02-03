/* -------------------------- */
/* Native (@) 2013 Stight.com */
/* -------------------------- */

var main = new Application();

var spline = new UILine(main, {
	class : "spline",
	left : 400,
	top : 200,
	width : 200,
	height : 200,
	background : "rgba(0, 200, 0, 0.1)",
	color : "black",
	lineWidth : 5,
	displayControlPoints : true,
	vertices : [
		42, 120,
		10, 10,
		150, 20,
		130, 80,
		70, 100
	]
});

var setSplineBounds = function(){
	var b = spline.boundingRect;

	bounds.left = b.left;
	bounds.top = b.top;
	bounds.width = b.width;
	bounds.height = b.height;

	spline.expand(
		Math.max(2*b.width, 200),
		Math.max(2*b.height, 200)
	);

};

var bounds = new UIElement(spline, {
	opacity : 0.0,
	background : "red"
});
bounds.sendToBack();

setSplineBounds();

spline.addEventListener("update", function(){
	setSplineBounds();
});


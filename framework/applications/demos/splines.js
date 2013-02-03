/* -------------------------- */
/* Native (@) 2013 Stight.com */
/* -------------------------- */

var main = new Application();

var spline = new UILine(main, {
	class : "spline",
	left : 40,
	top : 40,
	width : 700,
	height : 400,
	background : "rgba(0, 200, 0, 0.1)",
	color : "black",
	lineWidth : 5,
	displayControlPoints : true,
	vertices : [
		42, 305,
		100, 40,
		180, 20,
		350, 150,
		600, 380
	]
});

var setSplineBounds = function(){
	var b = spline.boundingRect;

	bounds.left = b.left;
	bounds.top = b.top;
	bounds.width = b.width;
	bounds.height = b.height;
};

var bounds = new UIElement(spline, {
	opacity : 0.25,
	background : "red"
});
bounds.sendToBack();

setSplineBounds();

spline.addEventListener("update", function(){
	setSplineBounds();
});


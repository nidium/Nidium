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

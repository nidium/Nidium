/* ------------------------+------------- */
/* Native Framework 2.0    | Falcon Build */
/* ------------------------+------------- */
/* (c) 2013 nidium.com - Vincent Fontaine */
/* -------------------------------------- */

/* -------------------------------------------------------------------------- */
load("libs/misc.lib.js");
/* -------------------------------------------------------------------------- */

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

var refreshBoundingElement = function(){
	var b = spline.boundingRect;
	bounds.left = b.left;
	bounds.top = b.top;
	bounds.width = b.width;
	bounds.height = b.height;
};

main.add("UIButton").move(10, 10).click(function(){

	var v = spline.getVertices();
	for (var i=0; i<v.length; i+=2){
		console.log(v[i], v[i+1]);
	}

	var b = spline.boundingRect;
	spline.shrink(spline.boundingRect);
	refreshBoundingElement();



	console.log("---------");
	var v = spline.getVertices();
	for (var i=0; i<v.length; i+=2){
		console.log(v[i], v[i+1]);
	}


});

var bounds = new UIElement(spline, {
	opacity : 0.0,
	background : "red"
});
bounds.sendToBack();
refreshBoundingElement();

spline.addEventListener("controlpointupdate", function(){
/*
	var b = spline.boundingRect;
	bounds.width = b.width;
	bounds.height = b.height;
	spline.expand(b.width, b.height);
*/
//	refreshBoundingElement();
});


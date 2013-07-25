/* ------------------------+------------- */
/* Native Framework 2.0    | Falcon Build */
/* ------------------------+------------- */
/* (c) 2013 Stight.com - Vincent Fontaine */
/* -------------------------------------- */

var main = new Application({
	id : "main",
	background : "#262722"
});

var	button = new UIButton(main, {
	id : "button",
	left : 966,
	top : 8,
	label : "Do It"
});

button.addEventListener("mouseup", function(e){
});

var	bigview = new UIView(main, {
	id : "bigview",
	left : 150,
	top : 58,
	width : 600,
	height : 400,
	background : "rgba(240, 80, 180, 0.5)",
	overflow : false,
	scrollable : true,
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
	scrollable : true
});

var	c0 = new UIView(view, {
	id : "c0",
	left : 250,
	top : 100,
	width : 40,
	height : 40,
	background : "#008800",
	position : "fixed"
});

var	c1 = new UIView(view, {
	id : "c1",
	left : 100,
	top : 50,
	width : 100,
	height : 150,
	background : "rgba(255, 255, 255, 0.7)",
	shadowBlur : 8,
	overflow : false,
	scrollable : true
});

var	c2 = new UIView(c1, {
	id : "c2",
	left : 10,
	top : 10,
	width : 20,
	height : 20,
	background : "#0088DD"
});

var	c22 = new UIView(c1, {
	id : "c22",
	left : 50,
	top : 50,
	width : 20,
	height : 20,
	background : "#0088DD"
});

var	c3 = new UIButton(c1, {
	id : "c3",
	left : 30,
	top : 260,
	label : "button",
	background : "#0088DD"
});

var	c4 = new UIView(view, {
	id : "c4",
	left : 280,
	top : 30,
	width : 20,
	height : 20,
	background : "#ff0088"
});

Native.layout.getElementsByTagName("UIView").each(function(){
	this.addEventListener("drag", function(e){
		this.left += e.xrel;
		this.top += e.yrel;
	});
});



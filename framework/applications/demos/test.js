/* -------------------------- */
/* Native (@) 2013 Stight.com */
/* -------------------------- */

var body = new Application({
	backgroundImage : "falcon/assets/back.png"
});

var	view = new UIView(body, {
	id : "view",
	left : 30,
	top : 30,
	width : 400,
	height : 250,
	background : "rgba(0, 0, 80, 0.5)",
	overflow : false,
	scrollbars : true
});

var	c0 = new UIView(view, {
	id : "c0",
	left : 250,
	top : 50,
	width : 40,
	height : 40,
	background : "#ff0000",
	cursor : "pointer"
});

var	c1 = new UIView(view, {
	id : "c1",
	left : 450,
	top : 300,
	width : 40,
	height : 40,
	background : "#008800",
	cursor : "pointer"
});

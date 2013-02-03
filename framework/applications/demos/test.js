/* -------------------------- */
/* Native (@) 2013 Stight.com */
/* -------------------------- */

var main = new Application();
main.className = "main";

var	myElements = [
	/* Tab 0 */ {label : "France", 			value : 5},
	/* Tab 1 */ {label : "Belgium", 		value : 7},
	/* Tab 2 */ {label : "Monaco", 			value : 9},
	/* Tab 3 */ {label : "United States",	value : 15, selected : true},
	/* Tab 4 */ {label : "Italy", 			value : 1},
	/* Tab 5 */ {label : "Spain", 			value : 3, background : "#202a15", color : "#ffffff"},
	/* Tab 6 */ {label : "Bulgaria",		value : 2, background : "#442033", color : "#ffffff"},
	/* Tab 7 */ {label : "Romania", 		value : 4},
	/* Tab 8 */ {label : "Sweden", 			value : 6},
	/* Tab 8 */ {label : "China", 			value : 8},
	/* Tab 8 */ {label : "Korea", 			value : 10},
	/* Tab 8 */ {label : "Luxembourg", 		value : 11},
	/* Tab 8 */ {label : "Switzerland", 	value : 12},
	/* Tab 9 */ {label : "Japan"}
];


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


var button = main.add("UIButton", {
	left : 200,
	top : 402,
	label : "Button XXXXXXX",
	class : "button"
}).addEventListener("mousedown", function(e){
	dropDownController.open();
});

var b = button.getBoundingRect();

document.addEventListener("mousewheel", function(e){
	button.angle += e.yrel;

	var b = button.getBoundingRect();
	
	viewXXX.left = b.x1;
	viewXXX.top = b.y1;
	viewXXX.width = b.x2 - b.x1;
	viewXXX.height = b.y2 - b.y1;
});

var b = button.getBoundingRect();
var viewXXX = main.add("UIView", {
	left : b.x1,
	top : b.y1,
	width : b.x2 - b.x1,
	height : b.y2 - b.y1,
	background : "rgba(80, 0, 0, 0.6)",
	class : "bounds"
});



var k1 = button.add("UIView", {
	left : 10,
	top : 10,
	width : 20,
	height : 20,
	background : "red",
	class : "k1"
});

var kk1 = k1.add("UIView", {
	left : 10,
	top : 10,
	width : 20,
	height : 20,
	background : "red",
	class : "kk1"
});

var k2 = button.add("UIView", {
	left : 10,
	top : 10,
	width : 20,
	height : 20,
	background : "red",
	class : "k2"
});

var k22 = k2.add("UIView", {
	left : 10,
	top : 10,
	width : 20,
	height : 20,
	background : "red",
	class : "k22"
});

var k3 = button.add("UIView", {
	left : 10,
	top : 10,
	width : 20,
	height : 20,
	background : "red",
	class : "k3"
});








var viewzzz = main.add("UIView", {
	left : 253,
	top : 413,
	width : 2,
	height : 2,
	background : "white",
	class : "centerPoint"
});





var viewzzfz = main.add("UIView", {
	left : 253,
	top : 413,
	width : 2,
	height : 2,
	background : "white",
	class : "centerPoint9"
});



var tgg = viewzzz.add("UIView", {
	left : 50,
	top : 50,
	width : 30,
	height : 30,
	background : "green",
	class : "greenChold"
});

var tgg2 = viewzzz.add("UIView", {
	left : 150,
	top : 150,
	width : 30,
	height : 30,
	background : "green",
	class : "greenKold"
});


var tgg3 = viewzzz.add("UIView", {
	left : 250,
	top : 250,
	width : 30,
	height : 30,
	background : "green",
	class : "greenFold"
});

var tggx = viewzzz.add("UIView", {
	left : 250,
	top : 250,
	width : 30,
	height : 30,
	background : "green",
	class : "greenzzz"
});




var lastlast = tgg.add("UIView", {
	left : 100,
	top : 100,
	width : 30,
	height : 20,
	background : "white",
	class : "last"
});


var lastlastlast = lastlast.add("UIView", {
	left : 100,
	top : 100,
	width : 30,
	height : 20,
	background : "white",
	class : "lastlastlast"
});



var kkkmrr = tgg2.add("UIView", {
	left : 100,
	top : 100,
	width : 30,
	height : 20,
	background : "white",
	class : "eep"
});


var tggr = viewzzz.add("UIView", {
	left : 250,
	top : 250,
	width : 30,
	height : 30,
	background : "green",
	class : "grerrrr"
});





var viewpre = main.add("UIView", {
	left : 10,
	top : 10,
	width : 30,
	height : 20,
	background : "white",
	class : "prePoint"
});




var rt = viewzzz.add("UIView", {
	left : 450,
	top : 450,
	width : 30,
	height : 30,
	background : "green",
	class : "rt"
});



var viewprfe = viewpre.add("UIView", {
	left : 10,
	top : 10,
	width : 30,
	height : 20,
	background : "white",
	class : "prePoint2"
});



var kkkmr2 = tgg3.add("UIView", {
	left : 100,
	top : 100,
	width : 30,
	height : 20,
	background : "white",
	class : "eep2"
});

var kkkmr4 = tgg3.add("UIView", {
	left : 100,
	top : 100,
	width : 30,
	height : 20,
	background : "white",
	class : "eep3"
});



var	dropDownController = main.add("UIDropDownController", {
	left : 538,
	top : 50,
	maxHeight : 200,
	name : "helloDrop",
	radius : 2,
	elements : myElements,
	background : "#191a18",
	selectedBackground : "#4D90FE",
	selectedColor : "#FFFFFF",
	class : "tabController"
});



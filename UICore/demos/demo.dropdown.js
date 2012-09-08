/* -------------------------- */
/* Native (@) 2012 Stight.com */
/* -------------------------- */


var document = new Application({background:"#262722"}),


	myTabs = [
		/* Tab 0 */ {label : "main.js", selected:true},
		/* Tab 1 */ {label : "core.js"},
		/* Tab 2 */ {label : "hello.js"},
		/* Tab 3 */ {label : "foo.cpp"},
		/* Tab 4 */ {label : "class.cpp"},
		/* Tab 5 */ {label : "opengl.cpp", background : "#202a15", color : "#ffffff"},
		/* Tab 6 */ {label : "2.js", background : "#442033", color : "#ffffff"},
		/* Tab 7 */ {label : "rotation.js"},
		/* Tab 8 */ {label : "scale.js"},
		/* Tab 9 */ {label : "native.inc.js"}
	];

var	tabController = document.createElement("UITabController", {
	name : "helloTabs",
	tabs : myTabs,
	background : "#191a18"
});



var	myElements = [
		/* Tab 0 */ {label : "France", 	value : 5},
		/* Tab 1 */ {label : "Belgium", 	value : 7 },
		/* Tab 2 */ {label : "Monaco", 	value : 9 },
		/* Tab 3 */ {label : "United States", 	value : 15, selected : true},
		/* Tab 4 */ {label : "Italy", 	value : 1 },
		/* Tab 5 */ {label : "Spain", 	value : 3, background : "#202a15", color : "#ffffff"},
		/* Tab 6 */ {label : "Bulgaria", 		value : 1, background : "#442033", color : "#ffffff"},
		/* Tab 7 */ {label : "Romania", value : 2},
		/* Tab 8 */ {label : "Sweden", 	value : 9},
		/* Tab 8 */ {label : "China", 	value : 9},
		/* Tab 8 */ {label : "Korea", 	value : 9},
		/* Tab 8 */ {label : "Luxembourg", 	value : 9},
		/* Tab 8 */ {label : "Switzerland", value : 9},
		/* Tab 9 */ {label : "Japan"}
	];

var	dropDownController = document.createElement("UIDropDownController", {
	x : 80,
	y : 50,
	name : "helloDrop",
	radius : 6,
	elements : myElements,
	background : "#191a18",
	selectedBackground : "#4D90FE",
	selectedColor : "#FFFFFF"
});


var win1 = document.createElement("UIWindow", {
	x : 50,
	y : 200,
	w : 400,
	h : 300,
	resizable : true,
	closeable : true
});

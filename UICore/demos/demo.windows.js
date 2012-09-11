/* -------------------------- */
/* Native (@) 2012 Stight.com */
/* -------------------------- */


var document = new Application({background:"#262722"}),

	win1 = document.createElement("UIWindow", {
		x : 150,
		y : 150,
		w : 400,
		h : 300,
		background : "#191a18",
		resizable : true,
		closeable : true,
		movable : true
	});

/*
for (var i=0; i<3; i++){
	win1.createElement("UIWindow", {
		x : 20+i*20,
		y : 40+i*15,
		w : 300,
		h : 200,
		background : "#191a18",
		resizable : true,
		closeable : true,
		movable : true
	});
}

win1.contentView.background = "#444444";

*/

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

var	dropDownController = win1.contentView.createElement("UIDropDownController", {
	x : 8,
	y : 8,
	name : "helloDrop",
	elements : myElements,
	background : "#191a18",
	selectedBackground : "#4D90FE",
	selectedColor : "#FFFFFF"
});

var input = win1.contentView.createElement("UITextInput", {
	x : 158,
	y : 8,
	w : 80,
	text : "Yes, Edit me !",
	background : "#191a18",
	color : "#222222"
});

/*
win1.handle.background = "rgba(255, 255, 0, 0.7)";
win1.handle.closeButton.background = "rgba(0, 255, 255, 0.7)";
win1.handle.closeButton.color = "rgba(0, 0, 0, 0.7)";
win1.contentView.background = "rgba(0, 255, 0, 0.2)";
*/


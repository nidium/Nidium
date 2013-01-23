/* -------------------------- */
/* Native (@) 2013 Stight.com */
/* -------------------------- */

var main = new Application();

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

var button = main.add("UIButton", {
	left : 180,
	top : 50,
	label : "Button"
}).addEventListener("mousedown", function(e){
	dropDownController.open();
});

var	dropDownController = main.add("UIDropDownController", {
	left : 238,
	top : 50,
	maxHeight : 200,
	name : "helloDrop",
	radius : 2,
	elements : myElements,
	background : "#191a18",
	selectedBackground : "#4D90FE",
	selectedColor : "#FFFFFF"
});


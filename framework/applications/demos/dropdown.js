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

Native.StyleSheet.add({
	radio : {
		left : 700,
		width : 200,
		name : "choice",
		fontType : "menlo",
		fontSize : 11,
		textShadowColor : "rgba(0, 0, 0, 0.4)",
		color : "#d0d0ff",
		lineWidth : 1,
		background : "rgba(0, 0, 0, 0.2)",
		borderColor : "rgba(0, 0, 0, 0.7)",
		borderWidth : 1,
		radius : 10,
		shadowBlur : 6,
		shadowColor : "rgba(0, 0, 0, 0.4)",
		shadowOffsetY : 3
	},

	red : {
		color : "red",
	},

	green : {
		color : "#e0ff60"
	},

	white : {
		color : "white"
	},

	black : {
		color : "black",
		background : "white",
		borderColor : null
	}
});


var	button = new UIButton(main, {
	left : 490,
	top : 50,
	label : "Do It"
});

var	radio1 = main.add("UIRadio", {
	top : 50,
	label : "Select this",
	value : "option1",
	selected : true,
	shadowBlur : 4,
	class : "radio"
});

var radio2 = main.add("UIRadio", {
	top : 78,
	value : "option2",
	label : "... or this",
	class : "radio red"
});

var radio3 = main.add("UIRadio", {
	top : 106,
	value : "option3",
	label : "... or this",
	class : "radio green"
});

var radio4 = main.add("UIRadio", {
	top : 134,
	value : "option4",
	label : "... or this",
	class : "radio white"
});

var radio5 = main.add("UIRadio", {
	top : 162,
	value : "option5",
	label : "... or this",
	class : "radio black"
});


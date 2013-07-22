/* -------------------------- */
/* Native (@) 2013 Stight.com */
/* -------------------------- */

var main = new Application({background:"#e0e0e0"});
main.className = "main";

var	myElements = [
	/* Tab 0 */ {label : "France", 			value : 5},
	/* Tab 1 */ {label : "Belgium", 		value : 7},
	/* Tab 2 */ {label : "Monaco", 			value : 9},
	/* Tab 3 */ {label : "United States",	value : 15, selected : true},
	/* Tab 4 */ {label : "Italy", 			value : 1},
	/* Tab 5 */ {label : "Spain", 			value : 3, background : "#202a15", color : "#000000"},
	/* Tab 6 */ {label : "Bulgaria",		value : 2, background : "#442033", color : "#000000"},
	/* Tab 7 */ {label : "Romania", 		value : 4},
	/* Tab 8 */ {label : "Sweden", 			value : 6},
	/* Tab 8 */ {label : "China", 			value : 8},
	/* Tab 8 */ {label : "Korea", 			value : 10},
	/* Tab 8 */ {label : "Luxembourg", 		value : 11},
	/* Tab 8 */ {label : "Switzerland", 	value : 12},
	/* Tab 9 */ {label : "Japan"}
];


var text = "In olden times when wishing still helped one when wishing still helped one, there lived a king X.";

var field = main.add("UITextField", {
	left : 10,
	top : 10,
	width : 960,
	fontSize : 12
});

field.value = "http://www.google.com/w/w/w/w/w/w/w/w/w/w/w/w/w/w/w/w/w/w/w/w/w/w/w/w/w/w/w/w/w/w/w/w/w/w/w/w/w/w/w/w/w/w/w/w/w/w/w/w";

var	button = new UIButton(main, {
	left : 980,
	top : 10,
	label : "Go"
}).click(function(){
	console.log(field.value);
});

var	dropDownController = main.add("UIDropDownController", {
	left : 538,
	top : 50,
	maxHeight : 200,
	name : "helloDrop",
	radius : 2,
	elements : myElements,
	background : '#333333',
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
		textShadowColor : "rgba(0, 0, 0, 0.05)",
		color : "#d0d0ff",
		lineWidth : 1,
		background : "rgba(0, 0, 0, 0.01)",
		borderColor : "rgba(0, 0, 0, 0.09)",
		borderWidth : 1,
		radius : 10
	},

	red : {
		color : "red",
	},

	green : {
		color : "#e0ff60"
	},

	white : {
		color : "#e0e0e0"
	},

	black : {
		color : "black",
		background : "white",
		borderColor : null
	}
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


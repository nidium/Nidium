/* ------------------------+------------- */
/* Native Framework 2.0    | Falcon Build */
/* ------------------------+------------- */
/* (c) 2013 nidium.com - Vincent Fontaine */
/* -------------------------------------- */

var main = new Application({
	class : "body"
});

var	myElements = [
	/* Tab 0 */ {label : "France", 			value : 5},
	/* Tab 1 */ {label : "Belgium", 		value : 7},
	/* Tab 2 */ {label : "Monaco", 			value : 9},
	/* Tab 3 */ {label : "United States",	value : 15, selected : true},
	/* Tab 4 */ {label : "Italy", 			value : 1},
	/* Tab 5 */ {label : "Spain", 			value : 3, class : "greenland"},
	/* Tab 6 */ {label : "Bulgaria",		value : 2, class : "greenland"},
	/* Tab 7 */ {label : "Romania", 		value : 4},
	/* Tab 8 */ {label : "Sweden", 			value : 6},
	/* Tab 8 */ {label : "China", 			value : 8},
	/* Tab 8 */ {label : "Korea", 			value : 10},
	/* Tab 8 */ {label : "Luxembourg", 		value : 11, disabled : true},
	/* Tab 8 */ {label : "Switzerland", 	value : 12},
	/* Tab 9 */ {label : "Japan", 			value : 13}
];

var field = main.add("UITextField", {
	left : 100,
	top : 50
});

field.value = "https://www.google.com/";
field.width = 430;

/*

var	lv = new UIListView(main, {
	left : 200,
	top : 100,
	width : 150,
	maxHeight : 198,
	radius : 2,
	elements : listItems
});

*/











var dateField = main.add("UITextField", {
	left : 100,
	top : 80,
	width : 84,
	placeholder : "dd/mm/yyyy",
	pattern : "(0[1-9]|1[0-9]|2[0-9]|3[01])/(0[1-9]|1[012])/[0-9]{4}"
});

dateField.addClass("selected");
dateField.id = "foobar";

var h = new UIToolTip(dateField, {
	label : "please enter a valid date"
});

h.enable();

field.addEventListener("submit", function(e){
	document.run(e.value);
});

dateField.addEventListener("submit", function(e){
	console.log("submit", e.match);
});





var	listItems = [
	/* option 0 */ {label : "France", 			value : 5},
	/* option 1 */ {label : "Belgium", 			value : 7},
	/* option 2 */ {label : "Monaco", 			value : 9, selectded:true},
	/* option 3 */ {label : "United States",	value : 15},
	/* option 4 */ {label : "Italy", 			value : 1},
	/* option 5 */ {label : "Spain", 			value : 3, class : "greenland"},
	/* option 6 */ {label : "Bulgaria",			value : 2, class : "greenland"},
	/* option 7 */ {label : "Romania", 			value : 4},
	/* option 8 */ {label : "Sweden", 			value : 6},
	/* option 8 */ {label : "China", 			value : 8},
	/* option 8 */ {label : "Korea", 			value : 10},
	/* option 8 */ {label : "Luxembourg", 		value : 11, disabled : true},
	/* option 8 */ {label : "Switzerland", 		value : 12},
	/* option 9 */ {label : "Japan", 			value : 13}
];


var	dropDownController = main.add("UIDropDownController", {
	name : "helloDrop",
	left : 538,
	top : 50,
	radius : 2
});

dropDownController.setOptions(listItems);

//dropDownController.value = 5;

dropDownController.addEventListener("change", function(e){
//	console.log("dropdown", e.value);
});

document.nss.add({
	".radio" : {
		left : 700,
		width : 200,
		autowidth : true,
		name : "choice",
		fontFamily : "menlo",
		fontSize : 11,
		background : "rgba(0, 0, 0, 0.05)",
		textShadowColor : "rgba(0, 0, 0, 0.1)",
		lineWidth : 1,
		borderColor : "rgba(0, 0, 0, 0.05)",
		borderWidth : 1,
		radius : 10
	},

	".red" : {
		color : "red",
	},

	".green" : {
		color : "#e0ff60"
	},

	".white" : {
		color : "#ffffff"
	},

	".transparent" : {
		color : "black",
		background : "rgba(255, 255, 255, 0.7)",
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
	label : "... or this"
});
radio3.className = "radio green";

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
	class : "radio transparent"
});

radio4.selected = true;

radio5.addEventListener("select", function(e){
	console.log(this.selected);
}, false);


/* ------- CHECKBOXES -------------- */

var	c0 = main.add("UICheckBox", {
	left : 700,
	top : 190,
	lineWidth : 0.5,
	fontSize : 9,
	label : "UICheckBox (radius = 0)",
	value : 5,
	selected : true,
	radius : 0
});

var	c1 = main.add("UICheckBox", {
	left : 700,
	top : 216,
	label : "default settings",
	value : 5,
	selected : true
});

var c2 = main.add("UICheckBox", {
	left : 700,
	top : 244,
	height : 25,
	value : 6,
	label : "UICheckBox (radius = 3)",
	background : "rgba(0, 0, 0, 0.1)",
	radius : 3,
	class : "red"
});

var c3 = main.add("UICheckBox", {
	left : 700,
	top : 272,
	height : 25,
	value : 7,
	label : "UICheckBox (radius = 12)",
	background : "rgba(255, 255, 255, 0.1)",
	radius : 12,
	class : "green"
});

c3.addEventListener("change", function(e){
	console.log(this.selected);
}, false);

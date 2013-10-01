/* ------------------------+------------- */
/* Native Framework 2.0    | Falcon Build */
/* ------------------------+------------- */
/* (c) 2013 nidium.com - Vincent Fontaine */
/* -------------------------------------- */

var main = new Application();
main.className = "body";

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


var	dropDownController = main.add("UIDropDownController", {
	left : 538,
	top : 50,
	name : "helloDrop",
	radius : 2,
	elements : myElements
});


dropDownController.value = 4;

dropDownController.addEventListener("change", function(e){
//	console.log("dropdown", e.value);
});



/*

var	button = new UIButton(main, {
	left : 980,
	top : 10,
	label : "Go"
}).click(function(){
	console.log(field.input.__startx, field.input.__starty);
});


document.nss.add({
	radio : {
		left : 700,
		width : 200,
		name : "choice",
		fontFamily : "menlo",
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
*/


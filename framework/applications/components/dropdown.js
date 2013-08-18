/* -------------------------- */
/* Native (@) 2013 Stight.com */
/* -------------------------- */

Native.StyleSheet.add({
	"#foobar" : {
		background : "rgba(255, 0, 0, 0.2)"
	},

	".selected" : {
		background : "green"
	},

	".greenland" : {
		background : "green",
		color : "#ffffff"
	},

	"UITextField" : {
		background : "white"
	}
});

var main = new Application();

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
	/* Tab 8 */ {label : "Luxembourg", 		value : 11},
	/* Tab 8 */ {label : "Switzerland", 	value : 12},
	/* Tab 9 */ {label : "Japan", 			value : 13}
];


var text = "In olden times when wishing still helped one when wishing still helped one, there lived a king X. In olden times when wishing still helped one when wishing still helped one, there lived a king X. In olden times when wishing still helped one when wishing still helped one, there lived a king X. In olden times when wishing still helped one when wishing still helped one, there lived a king X.";

var field = main.add("UITextField", {
	left : 100,
	top : 50
});

field.value = "https://www.google.fr/search?q=mac+osx+enlever+le+zoom&oq=mac+osx+enlever+le+zoom&aqs=chrome.0.69i57j0.5019j0&sourceid=chrome&ie=UTF-8#sclient=psy-ab&q=KMOD_GUI+sdl&oq=KMOD_GUI+sdl&gs_l=serp.3...45123.47082.2.47371.8.8.0.0.0.0.88.626.8.8.0....0.0..1c.1.20.psy-ab.inHNMKRk2vc&pbx=1&bav=on.2,or.r_cp.r_qf.&bvm=bv.49478099%2Cd.d2k%2Cpv.xjs.s.en_US.c75bKy5EQ0A.O&fp=d81ade728c71813c&biw=1512&bih=983";
field.color = "#660000";
field.width = 430;

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


dateField.addEventListener("contextmenu", function(e){
	console.log("context");
	e.preventMouseEvents();
});

field.addEventListener("submit", function(e){
	console.log("submit", e.value);
});

dateField.addEventListener("submit", function(e){
	console.log("submit", e.match);
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
	selectedColor : "#FFFFFF"
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
	echo(field.input.__startx, field.input.__starty);
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
*/

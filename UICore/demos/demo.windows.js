/* -------------------------- */
/* Native (@) 2012 Stight.com */
/* -------------------------- */


var main = new Application({background:"#262722"}),

	win1 = main.add("UIWindow", {
		x : 60,
		y : 60,
		w : 400,
		h : 300,
		name : 'Main Window',
		background : "rgba(0, 0, 0, 0.4)",
		color : '#ff0000',
		resizable : true,
		closeable : true,
		movable : true
	}),

	win2 = main.add("UIWindow", {
		x : main.w - 150,
		y : 30,
		w : 120,
		h : 100,
		closeable : true
	});

/* Add some customsization to Main Window */
win1.color = "rgba(0, 255, 0, 1)";
win1.handle.background = "rgba(0, 0, 0, 0.1)";
win1.handle.closeButton.background = "rgba(255, 255, 255, 0.8)";
win1.handle.closeButton.color = "rgba(0, 0, 0, 0.7)";
win1.contentView.background = "#ffffff";


var sampleText = "In olden times when wishing still helped one, there lived a king whose daughters were all beautiful; and the youngest was so beautiful that the sun itself, which has seen so much, was astonished whenever it shone in her face. Close by the king's castle lay a great dark forest, and under an old lime-tree in the forest was a well, and when the day was very warm.";
textView = main.add("UIText", {x:650, y:130, w:180, h:400, text:sampleText, background:"rgba(255, 255, 255, 1.00)"});


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

var	dropDownController = win1.contentView.add("UIDropDownController", {
	x : 8,
	y : 8,
	name : "helloDrop",
	elements : myElements,
	background : "#191a18",
	selectedBackground : "#4D90FE",
	selectedColor : "#FFFFFF"
});

var input = win1.contentView.add("UITextInput", {
	x : 158,
	y : 8,
	w : 80,
	text : "Yes, Edit me !",
	background : "#191a18",
	color : "#222222"
});


/* Add 4 children windows to main window */

var w = [];
for (var i=0; i<4; i++){
	w[i] = win1.add("UIWindow", {
		x : i*160,
		y : win1.h+20,
		w : 150,
		h : 150,
		background : "rgba(0, 20, 100, 0.5)",
		resizable : true,
		closeable : true,
		movable : true,
		name : 'Child ' + i
	});
	w[i].contentView.background = "rgba(255, 255, 255, 0.90)";
}

w[w.length-1].contentView.background = 'rgba(0, 0, 0, 0.85)';


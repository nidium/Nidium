/* -------------------------- */
/* Native (@) 2013 Stight.com */
/* -------------------------- */

__DEBUG_SHOW_LAYERS__ = true;
//__DEBUG_SHOW_ORDER__ = true;

var main = new Application({background:"#262722"});

var	myTabs = [
	/* Tab 0 */ {label : "main.js"},
	/* Tab 1 */ {label : "core.js"},
	/* Tab 2 */ {label : "hello.js", selected : true},
	/* Tab 3 */ {label : "foo.cpp"},
	/* Tab 4 */ {label : "class.cpp"},
	/* Tab 5 */ {label : "opengl.cpp", background : "#202a15", color : "#ffffff"},
	/* Tab 6 */ {label : "2.js", background : "#442033", color : "#ffffff"},
	/* Tab 7 */ {label : "rotation.js"},
	/* Tab 8 */ {label : "scale.js"},
	/* Tab 9 */ {label : "native.inc.js"}
];

var	tabController = main.add("UITabController", {
	name : "helloTabs",
	tabs : myTabs,
	background : "#191a18"
});

var removeButton = main.add("UIButton", {
		left : 20,
		top : 60,
		label:"Remove",
		background:"#4488EE",
		fontSize:10.5
	}),

	nextButton = main.add("UIButton", {
		left : 82, 
		top : 60, 
		label:"Next", 
		background:"#882266", 
		fontSize:10.5
	}),

	addButton = main.add("UIButton", {
		left : 130, 
		top : 85, 
		label:"Add", 
		background:"#668822", 
		fontSize:10.5
	}),

	b3 = main.add("UIButton", {
		left : 20, 
		top : 85, 
		label:"Tab 0, Position 0", 
		background:"#000000", 
		fontSize:10.5
	});

addButton.addEventListener("mouseclick", function(){
	var p = tabController.position;

	tabController.insertTab(p, {
		label : "Zombi Magic",
		background : "#202a15",
		color : "#ffffff"
	});
});

removeButton.addEventListener("mouseclick", function(){
	var p = tabController.position;
	tabController.removeTabAtPosition(p);
});

nextButton.addEventListener("mousedown", function(){
	tabController.selectNextTab();
});


tabController.addEventListener("tabselect", function(e){
	b3.label = "Tab " + e.index + ", Position " + e.position;
	main.background = tabController.tabs[e.index].background;
});

tabController.addEventListener("tabswap", function(e){
	b3.label = "Tab " + e.index + ", Position " + e.position;
});

/*

tabController.addEventListener("tabmove", function(e){
	var tab = e.index,
		positions = e.positions;

	echo("-- tab", tab, "moved to position", this.position);
	for (var i=0; i<positions.length; i++){
		echo("pos"+i, " --> tab", positions[i], " tabs["+i+"].position = ", this.tabs[i].position);
	}
	echo("");
});

tabController.addEventListener("tabclose", function(e){
	var tab = e.index,
		positions = e.positions;
	
	echo("-- tab", tab, "closed and no longer exists.");
	for (var i=0; i<positions.length; i++){
		echo("pos"+i, " --> tab", positions[i], " tabs["+i+"].position = ", this.tabs[i].position);
	}
	echo("");
});
*/



var NatBug = new Application({
	id : "NatBug",
	left : 0,
	top : 500,
	width : 1024,
	height : 268,
	background : "rgba(0, 0, 0, 0.20"
});

var lines = 0,
	col = 0,
	labels = {},
	values = {},
	attr = [
		"type", "id", "label", "name", "left", "top", "width",
		"height", "scale", "opacity", "selected", "fixed", "overflow",
		"opacity", "_absx", "_absy", "radius", "hasFocus", "isOnTop", "background", "color",
		"lineWidth", "lineHeight", "fontSize", "fontType", "textAlign",
		"shadowBlur"
	];

for (var l = 0; l<attr.length; l++){
	var a = attr[l],
		b1 = "rgba(0, 0, 0, 0.4)",
		b2 = "rgba(255, 255, 255, 0.05)";

	if (lines>=10){
		lines = 0;
		col++;
	}

	if (lines%2 == 0) {
		b1 = "rgba(0, 0, 0, 0.25)";
		b2 = "rgba(255, 255, 255, 0.075)";
	}

	labels[a] = NatBug.add("UILabel", {
		paddingLeft : 5,
		left : 5 + 260*col,
		top : 5 + 20*lines,
		width : 70,
		color : "#ffffff",
		background : b1,
		fontSize : 11,
		label : attr[l]
	});

	values[a] = NatBug.add("UILabel", {
		paddingLeft : 5,
		left : 78 + 260*col,
		top : 5 + 20*lines,
		width : 160,
		color : "#ffffff",
		background : b2,
		label : "",
		fontSize : 11,
		radius : 2
	});

	lines++;

}

window.onElementUnderPointer = function(e){
	if (e.y<NatBug._absy){
		for (var l = 0; l<attr.length; l++){
			var a = attr[l];
			values[a].label = this[a] + (this.parent ? " ("+this.parent[a]+")" : "");
		}
	}
};



















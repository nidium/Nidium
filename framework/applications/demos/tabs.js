/* ------------------------+------------- */
/* Native Framework 2.0    | Falcon Build */
/* ------------------------+------------- */
/* (c) 2013 Stight.com - Vincent Fontaine */
/* -------------------------------------- */

var main = new Application({id:"main", background:"#262722"});

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
		label : "Remove",
		background : "#4488EE",
		fontSize : 10.5
	}),

	nextButton = main.add("UIButton", {
		left : 82, 
		top : 60, 
		label : "Next", 
		background : "#882266", 
		fontSize : 10.5
	}),

	addButton = main.add("UIButton", {
		left : 140, 
		top : 85, 
		label : "Add"
	}),

	b3 = main.add("UIButton", {
		left : 20, 
		top : 85,
		label : "Tab 0, Position 0", 
		background : "#000000", 
		fontSize : 10.5
	});

addButton.addEventListener("mousedown", function(){
	var p = tabController.currentPosition;

	tabController.insertTab(p, {
		label : "Zombi Magic",
		color : "#ffffff"
	});
});

removeButton.addEventListener("mousedown", function(){
	var tab = tabController.getSelectedTab();
	tabController.removeTab(tab);
});

nextButton.addEventListener("mousedown", function(){
	tabController.selectNextTab();
});

tabController.addEventListener("tabselect", function(e){
	b3.label = "Tab " + e.index + ", Position " + e.pos;
	main.background = tabController.tabs[e.index].background;
});

tabController.addEventListener("tabswap", function(e){
	b3.label = "Tab " + e.index + ", Position " + e.pos;
});



var tab = tabController.getSelectedTab();
tab.label = "fsdfsdfsdffds"

/*

tabController.addEventListener("tabmove", function(e){
	var tab = e.index,
		elements = e.elements;

	echo("-- tab", tab, "moved to position", this.currentPosition);
	for (var i=0; i<elements.length; i++){
		echo("pos"+i, " --> tab", elements[i], " tabs["+i+"].pos = ", this.tabs[i].pos);
	}
	echo("");
});

tabController.addEventListener("tabclose", function(e){
	var tab = e.index,
		elements = e.elements;
	
	echo("-- tab", tab, "closed and no longer exists.");
	for (var i=0; i<elements.length; i++){
		echo("pos"+i, " --> tab", elements[i], " tabs["+i+"].pos = ", this.tabs[i].pos);
	}
	echo("");
});
*/








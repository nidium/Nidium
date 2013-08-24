/* ------------------------+------------- */
/* Native Framework 2.0    | Falcon Build */
/* ------------------------+------------- */
/* (c) 2013 Stight.com - Vincent Fontaine */
/* -------------------------------------- */

var main = new Application();

main.status = new UIStatus(main);
main.status.open();
main.status.progressBarColor = "rgba(210, 255, 60, 1)";

var	myTabs = [
	/* Tab 0 */ {label : "main.js", class : "tab"},
	/* Tab 1 */ {label : "core.js", class : "tab"},
	/* Tab 2 */ {label : "hello.js", selected : true, class : "tab"},
	/* Tab 3 */ {label : "foo.cpp", class : "tab"},
	/* Tab 4 */ {label : "class.cpp", class : "tab"},
	/* Tab 5 */ {label : "opengl.cpp", class : "tab"},
	/* Tab 6 */ {label : "2.js", class : "tab"},
	/* Tab 7 */ {label : "rotation.js", class : "tab"},
	/* Tab 8 */ {label : "scale.js", class : "tab"},
	/* Tab 9 */ {label : "native.inc.js", class : "tab"}
];

var	tabController = main.add("UITabController", {
	top : -2,
	name : "helloTabs",
	tabs : null,
	background : "rgba(25, 26, 24, 1)"
});

tabController.setTabs(myTabs);

var params = tabController.getDrawingBounds(),
	gradient = tabController.layer.context.createLinearGradient(
		params.x, params.y,
		params.x, params.y+params.h
	);

gradient.addColorStop(0.00, 'rgba(35, 36, 34, 1)');
gradient.addColorStop(0.95, 'rgba(27, 28, 26, 1)');
gradient.addColorStop(0.95, 'rgba(0, 0, 0, 1)');
gradient.addColorStop(0.98, 'rgba(0, 0, 0, 1)');
gradient.addColorStop(1.00, 'rgba(200, 200, 200, 0.1)');

tabController.background = gradient;


var tab = tabController.getSelectedTab();
tab.label = "fsdfsdfsdffds";

var removeButton = main.add("UIButton", {
		left : 20,
		top : 160,
		label : "Remove",
		background : "#4488EE",
		fontSize : 10.5
	}),

	nextButton = main.add("UIButton", {
		left : 82, 
		top : 160, 
		label : "Next", 
		background : "#882266", 
		fontSize : 10.5
	}),

	addButton = main.add("UIButton", {
		left : 140, 
		top : 185, 
		label : "Add"
	}),

	b3 = main.add("UIButton", {
		left : 20, 
		top : 185,
		label : "Tab 0, Position 0", 
		background : "#000000", 
		fontSize : 10.5
	});


addButton.click(function(){
	var tab = tabController.getSelectedTab(),
		position = tab ? tab.pos : 0;

	tabController.insertTab(position, {
		label : "Zombi Magic",
		color : "#ffffff"
	});
});

removeButton.click(function(){
	var tab = tabController.getSelectedTab();
	tabController.removeTab(tab);
});

nextButton.click(function(){
	tabController.selectNextTab();
});

tabController.addEventListener("tabselect", function(e){
	var tab = e.tab;
	b3.label = "Tab " + tab.index + ", Position " + tab.pos;
	main.background = tab.background;
});

tabController.addEventListener("tabswap", function(e){
	var tab = e.tab;
	b3.label = "Tab " + tab.index + ", Position " + tab.pos;
	return false;
});

tabController.addEventListener("tabmove", function(e){
	var tab = e.tab,
		elements = e.elements;

	console.log("-- tab", tab.index, "moved to position", this.currentPosition);
	for (var i=0; i<elements.length; i++){
		console.log("position "+i, " --> tab", elements[i], " tabs["+i+"].pos = ", this.tabs[i].pos);
	}
	console.log("");
});

tabController.addEventListener("tabbeforeclose", function(e){
	console.log("try to close tab ", e.tab.index);
});

tabController.addEventListener("tabclose", function(e){
	var tab = e.tab,
		elements = e.elements;
	
	console.log("-- tab closed.");
	for (var i=0; i<elements.length; i++){
		console.log("position "+i, " --> tab", elements[i], " tabs["+i+"].pos = ", this.tabs[i].pos);
	}
	console.log("");
});




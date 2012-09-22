/* -------------------------- */
/* Native (@) 2012 Stight.com */
/* -------------------------- */


var main = new Application({background:"#262722"}),

	myTabs = [
		/* Tab 0 */ {label : "main.js", selected:true},
		/* Tab 1 */ {label : "core.js"},
		/* Tab 2 */ {label : "hello.js"},
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
		x:20,
		y:60,
		label:"Remove",
		background:"#4488EE",
		fontSize:10.5
	}),

	nextButton = main.add("UIButton", {
		x:82, 
		y:60, 
		label:"Next", 
		background:"#882266", 
		fontSize:10.5
	}),

	addButton = main.add("UIButton", {
		x:130, 
		y:85, 
		label:"Add", 
		background:"#668822", 
		fontSize:10.5
	}),

	b3 = main.add("UIButton", {
		x:20, 
		y:85, 
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

	tabController.removeTab(p);
});

nextButton.addEventListener("mousedown", function(){
	tabController.selectNextTab();
});


tabController.addEventListener("tabselect", function(e){
	b3.label = "Tab " + e.tab + ", Position " + e.position;
	main.background = tabController.tabs[e.tab].background;
});

tabController.addEventListener("tabswap", function(e){
	var tab = e.tab,
		position = e.position;
	b3.label = "Tab " + e.tab + ", Position " + e.position;
});



/*


tabController.addEventListener("tabmove", function(e){
	var tab = e.tab,
		positions = e.positions;

	echo("-- tab", tab, "moved to position", this.position);
	for (var i=0; i<positions.length; i++){
		echo("pos"+i, " --> tab", positions[i], " tabs["+i+"].position = ", this.tabs[i].position);
	}
	echo("");
});

tabController.addEventListener("tabclose", function(e){
	var tab = e.tab,
		positions = e.positions;
	
	echo("-- tab", tab, "closed and no longer exists.");
	for (var i=0; i<positions.length; i++){
		echo("pos"+i, " --> tab", positions[i], " tabs["+i+"].position = ", this.tabs[i].position);
	}
	echo("");
});

*/

/*

var ypos = 10;

var sock = new Socket("127.0.0.1", 9000, {
	ssl : true,
	binary : true,
	charset : "iso8859-1" // latin | UTF8
});

sock.onconnect = function(){
    echo("JS : socket connected");
}

sock.onread = function(data){
	canvas.fillText(data, 10, ypos);
	ypos += 15;
}

*/


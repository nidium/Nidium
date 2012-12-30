/* -------------------------- */
/* Native (@) 2013 Stight.com */
/* -------------------------- */

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
	/* Tab 9 */ {label : "native.inc.js"},
	/* Tab 10 */ {label : "hello.inc.js"}
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
		label : "Add", 
		background : "#668822", 
		fontSize : 10.5
	}),

	b3 = main.add("UIButton", {
		left : 20, 
		top : 85, 
		label : "Tab 0, Position 0", 
		background : "#000000", 
		fontSize : 10.5
	});


addButton.addEventListener("mouseclick", function(){
	var p = tabController.position;

	tabController.insertTab(p, {
		label : "Zombi Magic",
		color : "#ffffff"
	});
});

removeButton.addEventListener("mouseclick", function(){
	var tab = tabController.getSelectedTab();
	tabController.removeTab(tab);
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


var tab = tabController.getSelectedTab();
/*
tab.addEventListener("change", function(e){
	console.log(e);
});
*/

tab.label = "New Name";
tab.fontType = "monospace";
tab.fontSize = 12;


//animate("opacity", removeButton.opacity, 0.1, 500);

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


var NatBug = {
	labels : {},
	values : {},

	props : [
		"id", "type", "label", "name", "left", "top", "width",
		"height", "paddingLeft", "paddingRight", "scale",
		"selected", "hover", "opacity", "_absx", "_absy", "radius",
		"hasFocus", "isOnTop", "background", "color",
		"lineWidth", "lineHeight", "fontSize", "fontType", "textAlign",
		"shadowBlur", "selection"
	],

	init : function(x=280, y=220){
		var self = this,
			lines = 0,
			col = 0;

		if (this.loaded) return false;

		this.loaded = true;

		this.application = new Application({
			id : "NatBug",
			left : x,
			top : y,
			width : 484,
			height : 328,
			background : "rgba(0, 80, 0, 0.65)",
			backgroundImage : "falcon/assets/back.png",
			radius : 8,
			opacity : 0.8
		});

		this.application.add("UILabel", {
			left : 418,
			top : 4,
			paddingLeft : 8,
			paddingRight : 8,
			height : 30,
			color : "#ffffff",
			background : "rgba(255, 255, 255, 0.02)",
			fontSize : 11,
			radius : 6,
			label : "NATBUG"
		});

		var	myTabs = [
			{
				label : "Properties",
				selected : true,
				background : "rgba(0, 0, 0, 0.0)",
				color : "#666666",
				fontSize : 13,
				closable : false
			},

			{
				label : "Network",
				background : "rgba(0, 0, 0, 0.3)",
				color : "#666666",
				fontSize : 13,
				closable : false
			}
		];

		this.tabController = this.application.add("UITabController", {
			name : "NatBugController",
			tabs : myTabs,
			left : 0,
			top : 3,
			height : 34
		});

		this.tabController.addEventListener("tabselect", function(e){
			var labels = Native.layout.getElementsByClassName("natbug_pane01");
			if (e.tab.label == "Properties") {
				labels.each(function(){
					this.fadeIn(150);
				});
			} else {
				labels.each(function(){
					this.fadeOut(150);
				});
			}
		});


		this.attachDragListener(this.tabController);
		this.attachDragListener(this.application);

		for (var l = 0; l<this.props.length; l++){
			var property = this.props[l],
				b1 = "rgba(0, 0, 0, 0.4)",
				b2 = "rgba(255, 255, 255, 0.05)";

			if (lines>=14){
				lines = 0;
				col++;
			}

			if (lines%2 == 0) {
				b1 = "rgba(0, 0, 0, 0.25)";
				b2 = "rgba(255, 255, 255, 0.075)";
			}

			this.createLine(property, lines, col, b1, b2);
			lines++;

		}

		this.show();
		Native.showFPS(true);
		Native.FPS.init();
		Native.__debugger = this.application;
		__DEBUG_SHOW_LAYERS__ = true;
		//__DEBUG_SHOW_ORDER__ = true;
	},

	createLine : function(property, line, col, b1, b2){
		this.labels[property] = this.application.add("UILabel", {
			paddingLeft : 5,
			left : 5 + 240*col,
			top : 44 + 20*line,
			width : 80,
			color : "#66aa66",
			background : b1,
			fontSize : 11,
			label : property
		});

		this.values[property] = this.application.add("UILabel", {
			paddingLeft : 5,
			left : 88 + 240*col,
			top : 44 + 20*line,
			width : 150,
			color : "#66aa66",
			background : b2,
			label : "",
			fontSize : 11,
			radius : 2
		});

		this.labels[property].addClass("natbug_pane01");
		this.values[property].addClass("natbug_pane01");

		this.attachDragListener(this.labels[property]);
		this.attachDragListener(this.values[property]);
	},

	attachDragListener : function(element){
		var self = this;
		element.addEventListener("drag", function(e){
			self.application.left += e.xrel;
			self.application.top += e.yrel;
		});
	},

	show : function(){
		var self = this;
		if (!this.loaded) return false;
		this.application.show();

		window.onElementUnderPointer = function(e){
			if (!self.application.isPointInside(e.x, e.y)){
				for (var l = 0; l<self.props.length; l++){
					var a = self.props[l];
					if (this[a] == undefined) {
						self.labels[a].color = "#66aa66";
						self.values[a].color = "#66aa66";
						self.values[a].label = 'n/a';
					} else {
						self.labels[a].color = "#ddffdd";
						self.values[a].color = "#ddffdd";
						self.values[a].label = this[a];
					}
					// + (this.parent ? " ("+this.parent[a]+")" : "");
				}
			}
		};
	},

	hide : function(){
		if (!this.loaded) return false;
		this.application.hide();
		delete(window.onElementUnderPointer);
	}

};

NatBug.init();














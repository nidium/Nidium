/* -------------------------- */
/* Native (@) 2012 Stight.com */
/* -------------------------- */

/* --- DEMO APP --------------------------------------------------------- */

var main = new Application({
		backgroundImage : "demos/assets/spheres.jpeg"
	}),

	myTabs = [
		/* Tab 0 */ {label : "main.js"},
		/* Tab 1 */ {label : "core.js"},
		/* Tab 2 */ {label : "hello.js"},
		/* Tab 3 */ {label : "foo.cpp", selected:true},
		/* Tab 4 */ {label : "class.cpp"},
		/* Tab 5 */ {label : "opengl.cpp", background:"#202a15", color:"#ffffff"},
		/* Tab 6 */ {label : "2.js", background:"#442033", color:"#ffffff"},
		/* Tab 7 */ {label : "rotation.js"},
		/* Tab 8 */ {label : "scale.js"},
		/* Tab 9 */ {label : "native.inc.js"}
	];

var	mainTabController = main.add("UITabController", {
		name : "masterTabs",
		tabs : myTabs,
		background : "#191a18"
	});


main.addEventListener("mousedblclick", function(){
	echo("dbl");
});


var win = main.add("UIWindow", {
	x : 280,
	y : 100,
	w : 300,
	h : 200,
	background : "rgba(0, 0, 0, 0.25)",
	resizable : true,
	closeable : true,
	movable : true
});



var template = "In olden times when wishing still helped one, there lived a king whose daughters were all beautiful; and the youngest was so beautiful that the sun itself, which has seen so much, was astonished whenever it shone in her face. Close by the king's castle lay a great dark forest, and under an old lime-tree in the forest was a well, and when the day was very warm, the king's child went out to the forest and sat down by the fountain; and when she was bored she took a golden ball, and threw it up on high and caught it; and this ball was her favorite plaything. Close by the king's castle lay a great dark forest, and under an old lime-tree in the forest was a well, and when the day was very warm, the king's child went out to the forest and sat down by the fountain; and when she was bored she took a golden ball, and threw it up on high and caught it; and this ball was her favorite plaything. [ɣ] s'écrit g. Quốc ngữ văn bản bằng tiếng Việt.";
var	sampleText = template.mul(10);

var	textView = main.add("UIText", {
	x : 633,
	y : 80,
	w : 380,
	h : 410,
	text : sampleText,
	offsetLeft : [0, 0, 0, 50, 50, 50, 50, 50, 50],
	offsetRight : [0, 154, 154, 154, 154, , , 230, 230, 230, 230, 230],
	lineHeight : 18,
	fontSize : 13,
	fontType : "arial",
	textAlign : "justify",
	background : "rgba(255, 255, 255, 1)",
	color : "#000000",
	overflow : false
});

var illustration0 = textView.add("UIView", {
	x : 0,
	y : 56,
	w : 48,
	h : 104,
	background : "#4455FF",
	fixed : false,
	overflow : true
});

var illustration1 = textView.add("UIView", {
	x : 230,
	y : 18,
	w : 150,
	h : 74,
	background : "#4455FF",
	backgroundImage : "demos/assets/video.png",
	fixed : false,
	overflow : true
});

var	illustration2 = textView.add("UIView", {
	x : 154,
	y : 126,
	w : 226,
	h : 90,
	background : "#ff5599",
	overflow : true,
	fixed : false
});

var	d = illustration2.add("UIButton", {
	x : -10, 
	y : -12, 
	label : "Fuck", 
	background : "#CC4488",
	radius : 6, 
	fontSize : 12
});

var icon = textView.add("UIView", {
	x : 160,
	y : 250,
	w : 100,
	h : 80,
	background : "#000033",
	overflow : false,
	fixed : true
});

var chld = icon.add("UIView", {
	x : 5,
	y : 45,
	w : 40,
	h : 50,
	background : "#999900"
});

var chld2 = chld.add("UIView", {
	x : 22,
	y : 4,
	w : 10,
	h : 80,
	background : "#0055DD"
});

chld2.addEventListener("drag", function(e){
	this.left = e.xrel + this.x;
	this.top = e.yrel + this.y;
}, true);



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

/*
var	drop1 = main.add("UIDropDownController", {
	x : 3,
	y : 38,
	name : "helloDrop1",
	elements : myElements,
	background : "#191a18",
	selectedBackground : "#4D90FE",
	selectedColor : "#FFFFFF"
});

var	drop2 = main.add("UIDropDownController", {
	x : 148,
	y : 38,
	name : "helloDrop2",
	elements : myElements,
	background : "#191a18",
	selectedBackground : "#4D90FE",
	selectedColor : "#FFFFFF"
});

var	drop3 = main.add("UIDropDownController", {
	x : 292,
	y : 38,
	name : "helloDrop3",
	elements : myElements,
	background : "#191a18",
	selectedBackground : "#4D90FE",
	selectedColor : "#FFFFFF"
});
*/



/* ---------------------------------------------------------------------- */

var NatBug = main.add("UIView", {
	id : "NatBug",
	x : 0,
	y : 500,
	w : 1024,
	h : 268,
	background : "#262722"
});

var lines = 0,
	col = 0,
	labels = {},
	values = {},
	attr = [
		"type", "id", "label", "name", "x", "y", "w", "h", "scale", "opacity",
		"selected", "fixed", "overflow", "opacity", "__x", "__y", "__w", "__h", "zIndex", "radius",
		"hasFocus", "isOnTop", "background", "color", "lineWidth", "lineHeight", "fontSize", "fontType", "textAlign", "shadowBlur"
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
		x : 5 + 240*col,
		y : 5 + 20*lines,
		w : 80,
		color : "#ffffff",
		background : b1,
		label : attr[l]
	});

	values[a] = NatBug.add("UILabel", {
		x : 88 + 240*col,
		y : 5 + 20*lines,
		w : 150,
		color : "#ffffff",
		background : b2,
		label : attr[l],
		radius : 2
	});

	lines++;

}

Native.mouseHook = function(e){
	if (e.y<NatBug._y){
		for (var l = 0; l<attr.length; l++){
			var a = attr[l];
			values[a].label = this[a];
		}

		/*
		var p = {
				x : this.__x,
				y : Math.min(this.__y, NatBug._y),
				w : this.__w,
				h : Math.min(this.__h, NatBug._y - this.__y) 
			},
			r = this.r+1;

		window.requestAnimationFrame = function(){
			canvas.setShadow(0, 0, 2, "rgba(255, 255, 255, 1)");
			canvas.roundbox(p.x, p.y, p.w, p.h, r, "rgba(0, 0, 0, 0.0)", "#ffffff");
			canvas.setShadow(0, 0, 4, "rgba(80, 190, 230, 1)");
			canvas.roundbox(p.x, p.y, p.w, p.h, r, "rgba(0, 0, 0, 0.0)", "#4D90FE");
			canvas.setShadow(0, 0, 5, "rgba(80, 190, 230, 1)");
			canvas.roundbox(p.x, p.y, p.w, p.h, r, "rgba(50, 80, 200, 0.05)", "#4D90FE");
			canvas.setShadow(0, 0, 0);
		};
		*/
	}

	e.stopPropagation();
};

DBT(function(){
	/*
	greenView.remove();
	Native.layout.unregister(main);
	Native.layout.register(mainTabController);
	*/
});


/* ---------------------------------------------------------------------- */


var	docButton1 = main.add("UIButton", {x:10, y:100, h:30, lineHeight:14, label:"docButton1", background:"#222222", radius:3, fontSize:14, selected:false}),
	docButton2 = main.add("UIButton", {x:10, y:140, label:"docButton2", background:"#4488CC", radius:3, fontSize:13, selected:false}),
	docButton3 = main.add("UIButton", {x:10, y:170, label:"docButton3", background:"#CC4488", radius:6, fontSize:12, selected:false}),
	docButton4 = main.add("UIButton", {x:10, y:200, label:"docButton4", background:"#8844CC", radius:6, fontSize:11, selected:false}),

	getTextButton = main.add("UIButton", {x:733, y:50, label:"Get Selection", background:"#0044CC", radius:6, fontSize:13, selected:false}),
	cutButton = main.add("UIButton", {x:858, y:50, label:"Cut", background:"#331111", radius:6, fontSize:13, selected:false}),
	copyButton = main.add("UIButton", {x:904, y:50, label:"Copy", background:"#113311", radius:6, fontSize:13, selected:false}),
	pasteButton = main.add("UIButton", {x:960, y:50, label:"Paste", background:"#111133", radius:6, fontSize:13, selected:false}),


	greenView = main.add("UIView", {id:"greenView", x:6, y:272, w:450, h:220, radius:6, background:"#fffffe", shadowBlur:26}),
	overlayView = greenView.add("UIView", {x:90, y:5, w:154, h:210, background:"rgba(0, 0, 0, 0.50)"}),
	davidButton = greenView.add("UIButton", {x:5, y:5, label:"David", background:"#338800"}),
	redViewButton1 = greenView.add("UIButton", {x:5, y:34, label:"RedView 1", background:"#338800", selected:true}),
	redViewButton2 = greenView.add("UIButton", {x:5, y:70, label:"RedView 2", background:"#338800", selected:false}),
	redViewButton3 = greenView.add("UIButton", {x:5, y:120, label:"RedView 3", background:"#338800", selected:false}),

	redViewButton4 = overlayView.add("UIButton", {x:5, y:5, label:"RedView 4", background:"#222222", selected:false}),
	radio1 = overlayView.add("UIRadio", {x:5, y:36, name:"choice", label:"Select this", selected:true}),
	radio2 = overlayView.add("UIRadio", {x:5, y:56, name:"choice", label:"... or this"});


var	tabController = greenView.add("UITabController", {
	name : "helloTabs",
	y : -32,
	tabs : [
		/* Tab 0 */ {label : "main.js", selected:true},
		/* Tab 1 */ {label : "core.js"},
		/* Tab 5 */ {label : "opengl.cpp", background:"#202a15", color:"#ffffff"},
		/* Tab 6 */ {label : "2.js", background:"#442033", color:"#ffffff"},
		/* Tab 7 */ {label : "rotation.js"}
	]
});





var	slider = main.add("UISliderController", {
	x : 908,
	y : 10,
	background : '#161712',
	color : 'rgba(255, 40, 210, 1)',
	disabled : false,
	radius : 2,
	min : 4,
	max : 18,
	value : 14
});

slider.addEventListener("change", function(value){
	mainTabController.overlap = value;
	mainTabController.resetTabs();
}, false);



/* ------------------------------------------------- */


/* ------------------------------------------------- */


var s = textView.setCaret(0, 540);

textView.addEventListener("textselect", function(s){
	//console.log(s);
	//echo('"' + textView.text.substr(s.offset, s.size) + '"');
	//this.select(false);
	//this.setCaret(s.offset, s.size);
	//this.getTextSelectionFromCaret(this.caret);
});

getTextButton.addEventListener("mousedown", function(e){
	echo(">>" + textView.getTextSelection() + "<<");
	echo("");
});

cutButton.addEventListener("mousedown", function(e){
	textView.cut();
});

copyButton.addEventListener("mousedown", function(e){
	textView.copy();
});

pasteButton.addEventListener("mousedown", function(e){
	textView.paste();
});

var brique = main.add("UIView", {x:150, y:150, w:60, h:60, radius:4, background:"rgba(255, 0, 0, 0.2)", draggable:true});

docButton1.addEventListener("mousedown", function(e){
	if (!this.toggle) {
		greenView.set("scale", 0, 150, function(){
			this.visible = false;
		}, Math.physics.quadInOut);
		this.toggle = true;
	} else {
		greenView.visible = true;
		greenView.set("scale", 1.5, 250, function(){}, Math.physics.elasticOut);
		this.toggle = false;
	}
});

docButton2.addEventListener("mousedown", function(e){
	var o = redViewButton4;

	o.transformOrigin = {
		x : o._x + o.w/2,
		y : o._y + o.h/2
	};

	console.log(o.transformOrigin);

	o.set("scale", 1.2, 350);
});

docButton3.addEventListener("mousedown", function(e){
	davidButton.set("scale", davidButton.scale+1, 150, function(){});
});

docButton4.addEventListener("mousedown", function(e){
	greenView.fadeOut(200);
});

//redViewButton4.scale = 2;

greenView.addEventListener("mousedown", function(e){
	this.bringToTop();
	e.stopPropagation();
}, false);

greenView.addEventListener("drag", function(e){
	this.left = e.xrel + this.x;
	this.top = e.yrel + this.y;

	this.transformOrigin = {
		x : e.x,
		y : e.y
	};

	var v = e.y>=(canvas.height/2) ? 1 : (e.y/(canvas.height/2));
	this.scale = v;
	this.opacity = v;
});

brique.addEventListener("dragstart", function(e){
	console.log("dragstart : " + e.target.id);
	e.dataTransfer.setData("Text", e.target.id);
});

brique.addEventListener("dragend", function(e){
	console.log("dragend brique");
});

docButton1.addEventListener("dragover", function(e){
	console.log("dragover");
});

docButton1.addEventListener("dragenter", function(e){
	console.log("dragenter " + e.source.id);
});

docButton1.addEventListener("dragleave", function(e){
	console.log("dragleave " + e.source.id);
});

docButton1.addEventListener("drop", function(e){
	console.log("source : " + e.source.id );
	console.log("target : " + e.target.id );

	console.log("using dataTransfer : " + e.dataTransfer.getData("Text") );
});






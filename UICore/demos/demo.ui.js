/* -------------------------- */
/* Native (@) 2012 Stight.com */
/* -------------------------- */

/* --- DEMO APP --------------------------------------------------------- */

var main = new Application(), /* {background : '#262722'} */

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
	],

	mainTabController = main.add("UITabController", {
		name : "masterTabs",
		tabs : myTabs,
		background : "#191a18"
	});


var template = "In olden times when wishing still helped one, there lived a king whose daughters were all beautiful; and the youngest was so beautiful that the sun itself, which has seen so much, was astonished whenever it shone in her face. Close by the king's castle lay a great dark forest, and under an old lime-tree in the forest was a well, and when the day was very warm, the king's child went out to the forest and sat down by the fountain; and when she was bored she took a golden ball, and threw it up on high and caught it; and this ball was her favorite plaything. Close by the king's castle lay a great dark forest, and under an old lime-tree in the forest was a well, and when the day was very warm, the king's child went out to the forest and sat down by the fountain; and when she was bored she took a golden ball, and threw it up on high and caught it; and this ball was her favorite plaything. [ɣ] s'écrit g. Quốc ngữ văn bản bằng tiếng Việt.";
var	sampleText = template.mul(3);

var	textView = main.add("UIText", {
		x : 733,
		y : 80,
		w : 280,
		h : 568,
		text : sampleText,
		lineHeight : 18,
		fontSize : 13,
		fontType : "arial",
		textAlign : "justify",
		background : "rgba(255, 255, 255, 1)",
		color : "#000000"
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


var	docButton1 = main.add("UIButton", {x:10, y:100, h:30, lineHeight:14, label:"docButton1", background:"#222222", radius:3, fontSize:14, selected:false}),
	docButton2 = main.add("UIButton", {x:10, y:140, label:"docButton2", background:"#4488CC", radius:3, fontSize:13, selected:false}),
	docButton3 = main.add("UIButton", {x:10, y:170, label:"docButton3", background:"#CC4488", radius:6, fontSize:12, selected:false}),
	docButton4 = main.add("UIButton", {x:10, y:200, label:"docButton4", background:"#8844CC", radius:6, fontSize:11, selected:false}),
	docButton5 = main.add("UIButton", {x:10, y:230, label:"docButton5", background:"#4400CC", radius:6, fontSize:10, selected:false}),
	docButton6 = main.add("UIButton", {x:10, y:260, h:40, lineHeight:40, label:"docButton6", background:"#0044CC", radius:6, fontSize:9, selected:false}),

	getTextButton = main.add("UIButton", {x:733, y:50, label:"Get Selection", background:"#0044CC", radius:6, fontSize:13, selected:false}),
	cutButton = main.add("UIButton", {x:858, y:50, label:"Cut", background:"#331111", radius:6, fontSize:13, selected:false}),
	copyButton = main.add("UIButton", {x:904, y:50, label:"Copy", background:"#113311", radius:6, fontSize:13, selected:false}),
	pasteButton = main.add("UIButton", {x:960, y:50, label:"Paste", background:"#111133", radius:6, fontSize:13, selected:false}),


	greenView = main.add("UIView", {id:"greenView", x:140, y:480, w:450, h:220, radius:6, background:"#fffffe", shadowBlur:26}),
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

/* ------------------------------------------------- */


DBT(function(){
	echo(line.controlPoint2.isVisible());
});

var s = textView.setCaret(70, 50);

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

var line = overlayView.add("UILine", {x1:170, y1:16, x2:350, y2:200, split:"quadratic", color:"#ff0000", lineWidth:4});
var brique = main.add("UIView", {x:150, y:150, w:60, h:60, radius:4, background:"rgba(255, 0, 0, 0.2)", draggable:true});

docButton1.addEventListener("mousedown", function(e){
	if (!this.toggle) {
		greenView.set("scale", 0, 150, function(){
			this.visible = false;
		}, FXAnimation.easeInOutQuad);
		this.toggle = true;
	} else {
		greenView.visible = true;
		greenView.set("scale", 1.5, 250, function(){}, FXAnimation.easeOutElastic);
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

docButton5.addEventListener("mousedown", function(e){
	canvas.animate = false;
	canvas.blur(0, 0, 1024, 768, 2);
});

docButton6.addEventListener("mousedown", function(e){
	canvas.animate = true;
	greenView.hide();
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






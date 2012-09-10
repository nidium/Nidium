/* -------------------------- */
/* Native (@) 2012 Stight.com */
/* -------------------------- */

/* --- DEMO APP --------------------------------------------------------- */

		var gdGreen = canvas.createLinearGradient(0, 0, 0, 400);
		gdGreen.addColorStop(0.00,'#44ff77');
		gdGreen.addColorStop(0.60,'#228855');
		gdGreen.addColorStop(1.00,'#ffffff');

		var gdDark = canvas.createLinearGradient(0, 0, 0, 150);
		gdDark.addColorStop(0.00,'#222222');
		gdDark.addColorStop(0.25,'#444444');
		gdDark.addColorStop(0.60,'#222222');
		gdDark.addColorStop(1.00,'#000000');

		var gdBackground = canvas.createLinearGradient(0, 0, 0, window.height);
		gdBackground.addColorStop(0.00,'#444444');
		gdBackground.addColorStop(1.00,'#111111');

var sampleText = "In olden times when wishing still helped one, there lived a king whose daughters were all beautiful; and the youngest was so beautiful that the sun itself, which has seen so much, was astonished whenever it shone in her face. Close by the king's castle lay a great dark forest, and under an old lime-tree in the forest was a well, and when the day was very warm, the king's child went out to the forest and sat down by the fountain; and when she was bored she took a golden ball, and threw it up on high and caught it; and this ball was her favorite plaything. Close by the king's castle lay a great dark forest, and under an old lime-tree in the forest was a well, and when the day was very warm, the king's child went out to the forest and sat down by the fountain; and when she was bored she took a golden ball, and threw it up on high and caught it; and this ball was her favorite plaything. ";



/* --------------- */

var document = new Application({background:"#262722"}),

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

	mainTabController = document.createElement("UITabController", {
		name : "masterTabs",
		tabs : myTabs,
		background : "#191a18"
	});

	textView = document.createElement("UIText", {x:743, y:80, w:280, h:568, text:sampleText, background:"rgba(255, 255, 255, 1.00)"}),

	docButton1 = document.createElement("UIButton", {x:10, y:110, label:"docButton1", background:"#222222", radius:3, fontSize:14, selected:false}),
	docButton2 = document.createElement("UIButton", {x:10, y:140, label:"docButton2", background:"#4488CC", radius:3, fontSize:13, selected:false}),
	docButton3 = document.createElement("UIButton", {x:10, y:170, label:"docButton3", background:"#CC4488", radius:6, fontSize:12, selected:false}),
	docButton4 = document.createElement("UIButton", {x:10, y:200, label:"docButton4", background:"#8844CC", radius:6, fontSize:11, selected:false}),
	docButton5 = document.createElement("UIButton", {x:10, y:230, label:"docButton5", background:"#4400CC", radius:6, fontSize:10, selected:false}),
	docButton6 = document.createElement("UIButton", {x:10, y:260, label:"docButton6", background:"#0044CC", radius:6, fontSize:9, selected:false}),

	greenView = document.createElement("UIView", {id:"greenView", x:340, y:180, w:450, h:220, radius:6, background:"#ffffff", shadowBlur:26}),
	overlayView = greenView.createElement("UIView", {x:90, y:5, w:154, h:210, background:"rgba(0, 0, 0, 0.50)"}),
	davidButton = greenView.createElement("UIButton", {x:5, y:5, label:"David", background:"#338800"}),
	redViewButton1 = greenView.createElement("UIButton", {x:5, y:34, label:"RedView 1", background:"#338800", selected:true}),
	redViewButton2 = greenView.createElement("UIButton", {x:5, y:70, label:"RedView 2", background:"#338800", selected:false}),
	redViewButton3 = greenView.createElement("UIButton", {x:5, y:120, label:"RedView 3", background:"#338800", selected:false}),

	redViewButton4 = overlayView.createElement("UIButton", {x:5, y:5, label:"RedView 4", background:"#222222", selected:false}),
	radio1 = overlayView.createElement("UIRadio", {x:5, y:36, name:"choice", label:"Select this", selected:true}),
	radio2 = overlayView.createElement("UIRadio", {x:5, y:56, name:"choice", label:"... or this"});


var	tabController = greenView.createElement("UITabController", {
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



var line = overlayView.createElement("UILine", {x1:20, y1:110, x2:100, y2:180, split:"quadratic", color:"#ff0000"});
var brique = document.createElement("UIView", {x:150, y:150, w:60, h:60, radius:4, background:"rgba(255,0,0,0.2)", draggable:true});


var win = document.createElement("UIWindow", {
	x : 80,
	y : 100,
	w : 300,
	h : 200,
	background : "#191a18",
	resizable : true,
	closeable : true,
	movable : true
});

docButton1.addEventListener("mousedown", function(e){
	if (!this.toggle) {
		greenView.bounceScale(0.0, 150, function(){
			this.visible = false;
		}, FXAnimation.easeInOutQuad);
		this.toggle = true;
	} else {
		this.visible = true;
		greenView.bounceScale(2, 300, function(){
			this.scale = 2;
		}, FXAnimation.easeOutElastic);
		this.toggle = false;
	}
});

docButton2.addEventListener("mousedown", function(e){
	redViewButton4.g = {
		x : -redViewButton4.w/2,
		y : -redViewButton4.h/2
	};
	redViewButton4.bounceScale(1.2, 350, function(){});
});

docButton3.addEventListener("mousedown", function(e){
	davidButton.bounceScale(davidButton.scale+1, 150, function(){});
});

docButton4.addEventListener("mousedown", function(e){
	greenView.fadeOut(200, function(){});
});


var blurCache = false;

docButton5.addEventListener("mousedown", function(e){
	canvas.animate = false;

	//if (!blurCache) {
		blurCache = canvas.fastblur(0, 0, 1024, 768, 2);
	//} else {
	//	context.putImageData(blurCache, 0, 0);
	//}
	//canvas.fillStyle = "rgba(255, 255, 255, 0.1)";
	//canvas.fillRect(0, 0, 1024, 768);
});

docButton6.addEventListener("mousedown", function(e){
	canvas.animate = true;



	for (var child in layout.elements){
		let nodes = layout.elements;
		echo(
			nodes[child]._zIndex, 
			nodes[child]._rIndex, 
			nodes[child].zIndex, 
			"("+nodes[child].label+")",
			nodes[child].type,
			nodes[child].id
		);
	}


});

//greenView.zIndex = 800;

/*
var t = new Thread(function(){
	this.send();
});

t.onmessage = function(e){
	e.data;
};
*/


/*
var l = +new Date();
setTimeout(function() {
    var c = +new Date();
    echo("Timer", c-l);
    l = c;
}, 1000, true, true);
*/


//redViewButton4.scale = 2;

greenView.addEventListener("mousedown", function(e){
	//console.log("start");
	this.bringToTop();
});

greenView.addEventListener("drag", function(e){
	this.left = e.xrel + this.x;
	this.top = e.yrel + this.y;
});


/*
document.addEventListener("mousedown", function(e){
	this.createElement("UIButton", {x:e.x, y:e.y, label:"Default", background:"#222222", radius:3, selected:false})
});
*/


/*


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

greenView.addEventListener("mouseover", function(e){
	console.log("over");
});
greenView.addEventListener("mouseout", function(e){
	console.log("out");
});



*/



/*

window.onmousemove = function(x, y){

	document.left = x - 30;
	document.top = y - 30; 

};






for (var i=0; i<5; i++){
	var x = Math.round(Math.random()*1024);
	var y = Math.round(Math.random()*768);
	document.createElement("UIView", {x:x, y:y, w:90, h:90, radius:10, background:"rgba(255,255,0,0.5)"});
}

*/

/*

var file = new DataStream("http://www.google.fr/logo.gif");
file.buffer(4096);
file.chunk(2048);
file.seek(154555);
file.ondata = function(data){
	for (var i=0; i<data.length; i++){
		echo(data[i]);
	}
};
file.open();
file.onclose();

*/


/*
document.hide();
document.show();
myButton1.hide();
myButton1.show();
docButton1.show();
*/




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

var template = "In olden times when wishing still helped one, there lived a king whose daughters were all beautiful; and the youngest was so beautiful that the sun itself, which has seen so much, was astonished whenever it shone in her face. Close by the king's castle lay a great dark forest, and under an old lime-tree in the forest was a well, and when the day was very warm, the king's child went out to the forest and sat down by the fountain; and when she was bored she took a golden ball, and threw it up on high and caught it; and this ball was her favorite plaything. Close by the king's castle lay a great dark forest, and under an old lime-tree in the forest was a well, and when the day was very warm, the king's child went out to the forest and sat down by the fountain; and when she was bored she took a golden ball, and threw it up on high and caught it; and this ball was her favorite plaything. [ɣ] s'écrit g. Quốc ngữ văn bản bằng tiếng Việt.";
	st = [],
	sampleText = '';
for (var t=0; t<100; t++){
	st.push(template);
}
sampleText = st.join('');


/* --------------- */

var main = new Application({background:"#262722"}),

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


var	textView = main.add("UIText", {x:643, y:80, w:280, h:568, text:sampleText, lineHeight:18, fontSize:13, textAlign:"justify", background:"rgba(255, 255, 255, 1.00)", color:"#000000"}),

	docButton1 = main.add("UIButton", {x:10, y:110, label:"docButton1", background:"#222222", radius:3, fontSize:14, selected:false}),

	//docButton1 = new UIButton({x:10, y:110, label:"docButton1", background:"#222222", radius:3, fontSize:14, selected:false});

	docButton2 = main.add("UIButton", {x:10, y:140, label:"docButton2", background:"#4488CC", radius:3, fontSize:13, selected:false}),
	docButton3 = main.add("UIButton", {x:10, y:170, label:"docButton3", background:"#CC4488", radius:6, fontSize:12, selected:false}),
	docButton4 = main.add("UIButton", {x:10, y:200, label:"docButton4", background:"#8844CC", radius:6, fontSize:11, selected:false}),
	docButton5 = main.add("UIButton", {x:10, y:230, label:"docButton5", background:"#4400CC", radius:6, fontSize:10, selected:false}),
	docButton6 = main.add("UIButton", {x:10, y:260, label:"docButton6", background:"#0044CC", radius:6, fontSize:9, selected:false}),

	getTextButton = main.add("UIButton", {x:743, y:50, label:"Get Text Selection", background:"#0044CC", radius:6, fontSize:13, selected:false}),


	greenView = main.add("UIView", {id:"greenView", x:140, y:480, w:450, h:220, radius:6, background:"#ffffff", shadowBlur:26}),
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

textView.caret = {
	x1 : 5,
	y1 : 2,
	x2 : 40,
	y2 : 4
};

/*
var t = +new Date(),
	s;
for (var z=0; z<2; z++){
	s = textView.getTextSelectionFromCaret(textView.caret);
}
echo((+new Date()-t));

console.log(s);
*/


var line = overlayView.add("UILine", {x1:20, y1:110, x2:100, y2:180, split:"quadratic", color:"#ff0000"});
var brique = main.add("UIView", {x:150, y:150, w:60, h:60, radius:4, background:"rgba(255,0,0,0.2)", draggable:true});


var win = main.add("UIWindow", {
	x : 280,
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


getTextButton.addEventListener("mousedown", function(e){
	echo(">>" + textView.selection.text + "<<");
	echo("");
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
main.addEventListener("mousedown", function(e){
	this.add("UIButton", {x:e.x, y:e.y, label:"Default", background:"#222222", radius:3, selected:false})
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

	main.left = x - 30;
	main.top = y - 30; 

};






for (var i=0; i<5; i++){
	var x = Math.round(Math.random()*1024);
	var y = Math.round(Math.random()*768);
	main.add("UIView", {x:x, y:y, w:90, h:90, radius:10, background:"rgba(255,255,0,0.5)"});
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
main.hide();
main.show();
myButton1.hide();
myButton1.show();
docButton1.show();
*/




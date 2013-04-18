/* -------------------------- */
/* Native (@) 2013 Stight.com */
/* -------------------------- */

var body = new Application({
	backgroundImage : "falcon/assets/back.png"
});

var text = "In olden times when wishing still helped one, there lived a king whose daughters were all beautiful; and the youngest was so beautiful that the sun itself, which has seen so much, was astonished whenever it shone in her face. Close by the king's castle lay a great dark forest, and under an old lime-tree in the forest was a well, and when the day was very warm, the king's child went out to the forest and sat down by the fountain; and when she was bored she took a golden ball, and threw it up on high and caught it; and this ball was her favorite plaything. Close by the king's castle lay a great dark forest, and under an old lime-tree in the forest was a well, and when the day was very warm, the king's child went out to the forest and sat down by the fountain; and when she was bored she took a golden ball, and threw it up on high and caught it; and this ball was her favorite plaything. [ɣ] s'écrit g. Quốc ngữ văn bản bằng tiếng Việt.";

var textView = body.add("UIView", {
	left : 500,
	top : 80,
	width : 300,
	height : 200,
	background : "white",
	overflow : false,
	scrollbars : true
});

var textInput = textView.add("UITextInput", {
	left : 0,
	top : 0, 
	fontSize : 12,
	lineHeight : 18,
	text : text,
	textAlign : "justify",
	editable : true
});

var	view = new UIView(body, {
	id : "view",
	left : 30,
	top : 30,
	width : 400,
	height : 250,
	background : "rgba(0, 0, 80, 0.5)",
	overflow : false,
	scrollbars : true
});

var	c0 = new UIView(view, {
	id : "c0",
	left : 250,
	top : 50,
	width : 40,
	height : 40,
	background : "#ff0000",
	cursor : "pointer",
	position : "fixed"
});

var	kevin = new UIView(c0, {
	id : "c0",
	left : 20,
	top : 20,
	width : 40,
	height : 40,
	background : "blue",
	cursor : "pointer"
});

kevin.drag(function(e){
	this.left += e.xrel;
	this.top += e.yrel;
});

var	c1 = new UIView(textView, {
	id : "c1",
	left : 450,
	top : 300,
	width : 40,
	height : 40,
	background : "#008800",
	cursor : "pointer"
});


c0.click(function(){

	window.openFileDialog(["png", "jpg", "jpeg"], function(res){
		for (var i=0; i<res.length; i++){
			echo(res[i])
		}
		var img = new Image();
		img.src = res[0];
		img.onload = function(){
			view.setBackgroundImage(img);
			view.angle = 15;
		}
	});

});
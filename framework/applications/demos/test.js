/* -------------------------- */
/* Native (@) 2013 Stight.com */
/* -------------------------- */

var body = new Application();
body.className = "body";

var div = new UIView();
div.left = 250;
div.top = 100;
div.width = 300;
div.height = 250;
div.background = "white";
div.textContent = "the brown fox sugar";
body.addChild(div);


div.add("UIButton");
div.add("UIButton");


var span = new UIElement();
span.width = 80;
span.background = "red";
span.textContent = "lipsum natsu";
div.addChild(span);

div.add("UIButton");
div.add("UIButton");
div.add("UIButton");



body.add("UIButton").click(function(){
	window.title = Math.random() + "";
});


/* se mettre à la place de Google pour le référencement */


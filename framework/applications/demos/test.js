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


var k1 = div.add("UIButton");
var k2 = div.add("UIButton");


var span = new UIElement();
span.width = 80;
span.background = "red";
span.textContent = "lipsum natsu";
div.addChild(span);

var a = div.add("UIButton");
var b = div.add("UIButton");
var c = div.add("UIButton");

body.add("UIButton").click(function(){
	window.title = Math.random() + "";
	echo(body.isAncestor(div));
	echo(div.isAncestor(body));
	echo(k2.nodeIndex)
});


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
div.background = "black";
div.textContent = "the brown fox sugar";
body.addChild(div);


var k1 = div.add("UIButton");
var k2 = div.add("UIButton");


var span = new UIElement();
span.width = 80;
span.background = "red";
span.textContent = "lipsum natsu";
div.addChild(span);

var a = div.add("UIButton").move(0, 0);
var b = div.add("UIButton").move(15, 15);
var b = div.add("UIButton").move(30, 30);
var c = new UIButton().move(-15, -15);
c.background = "red";

body.add("UIButton").click(function(){
	window.title = Math.random() + "";
	echo(c.parent);
	div.insertChildAtIndex(c, 0);
	echo(c.parent.id);
});


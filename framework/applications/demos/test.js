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
div.className = "div";
body.addChild(div);

echo(div.layer.width, "contentWidth", div.contentWidth);




var t1 = new TextNode("The quick brown ");
var t2 = new TextNode("fox");
var t3 = new TextNode(" jumps over the lazy ");
var t4 = new TextNode("dog");
var t5 = new TextNode(".     ");
var t6 = new TextNode("The earliest known appearance of the phrase is from The Michigan School Moderator.");

div.addChild(t1);
div.addChild(t2);
div.addChild(t3);
div.addChild(t4);
div.addChild(t5);
//div.addChild(t6);


var t41 = new TextNode(" jumps over the lazy ");

body.add("UIButton").click(function(){
	echo(div.contentWidth);
//	t4.remove();
});

t2.color = "red";
t4.color = "#ffdd88";

t4.cursor = "pointer";
t4.addEventListener("mouseover", function(e){
	this.hover = true;
});

t4.addEventListener("mouseout", function(e){
	this.hover = false;
});

/*
var h = new UIToolTip(t4, {
	label : "Click here to see the Quick Brown Fox"
});
h.enable();
h.show();
h.visible = false;
*/


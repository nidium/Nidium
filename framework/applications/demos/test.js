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
//body.addChild(div);

div.add("UIButton");


body.add("UIButton").click(function(){
	echo(div.rooted);
	body.addChild(div);
	echo(div.rooted);
});


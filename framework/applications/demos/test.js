/* -------------------------- */
/* Native (@) 2013 Stight.com */
/* -------------------------- */


var body = new Application({
	backgroundImage : "falcon/assets/black.png",
	class : "body"
});

var div = new UIView(body, {
	left : 250,
	top : 100,
	width : 300,
	height : 250,
	innerWidth : 370,
	fontSize : 13,
	background : "rgba(255, 255, 255, 0.1)",
	textAlign : "justify",
	overflow : false,
	scrollbars : true,
	class : "div"
});

var extenser = div.add("UIElement", {
	top : 180,
	left : 0,
	width : 380,
	height : 10,
	background : "white",
	opacity : 0
});

var t1 = new TextNode("The quick brown ");
var t2 = new TextNode("fox");
var t3 = new TextNode(" jumps over the lazy ");
var t4 = new TextNode("dog");
var t5 = new TextNode(". ");
var t6 = new TextNode("The earliest known appearance of the phrase");
var t7 = new TextNode(" is from The Michigan School Moderator. The earliest known appearance of the phrase is from The Michigan School Moderator. The earliest known appearance of the phrase is from The Michigan School Moderator. The earliest known appearance of the phrase is from The Michigan School Moderator. The earliest known appearance of the phrase is from The Michigan School Moderator. The earliest known appearance of the phrase is from The Michigan School Moderator. The earliest known appearance of the phrase is from The Michigan School Moderator. The earliest known appearance of the phrase is from The Michigan School Moderator. The earliest known appearance of the phrase is from The Michigan School Moderator. The earliest known appearance of the phrase is from The Michigan School Moderator. The earliest known appearance of the phrase is from The Michigan School Moderator. The earliest known appearance of the phrase is from The Michigan School Moderator. The earliest known appearance of the phrase is from The Michigan School Moderator. The earliest known appearance of the phrase is from The Michigan School Moderator. The earliest known appearance of the phrase is from The Michigan School Moderator. The earliest known appearance of the phrase is from The Michigan School Moderator. The earliest known appearance of the phrase is from The Michigan School Moderator. The earliest known appearance of the phrase is from The Michigan School Moderator. The earliest known appearance of the phrase is from The Michigan School Moderator. The earliest known appearance of the phrase is from The Michigan School Moderator. The earliest known appearance of the phrase is from The Michigan School Moderator. The earliest known appearance of the phrase is from The Michigan School Moderator. The earliest known appearance of the phrase is from The Michigan School Moderator. The earliest known appearance of the phrase is from The Michigan School Moderator. The earliest known appearance of the phrase is from The Michigan School Moderator. The earliest known appearance of the phrase is from The Michigan School Moderator. The earliest known appearance of the phrase is from The Michigan School Moderator. The earliest known appearance of the phrase is from The Michigan School Moderator. The earliest known appearance of the phrase is from The Michigan School Moderator. The earliest known appearance of the phrase is from The Michigan School Moderator. The earliest known appearance of the phrase is from The Michigan School Moderator. The earliest known appearance of the phrase is from The Michigan School Moderator. The earliest known appearance of the phrase is from The Michigan School Moderator. The earliest known appearance of the phrase is from The Michigan School Moderator. The earliest known appearance of the phrase is from The Michigan School Moderator. The earliest known appearance of the phrase is from The Michigan School Moderator. The earliest known appearance of the phrase is from The Michigan School Moderator. The earliest known appearance of the phrase is from The Michigan School Moderator. The earliest known appearance of the phrase is from The Michigan School Moderator. The earliest known appearance of the phrase is from The Michigan School Moderator. The earliest known appearance of the phrase is from The Michigan School Moderator. The earliest known appearance of the phrase is from The Michigan School Moderator. The earliest known appearance of the phrase is from The Michigan School Moderator. The earliest known appearance of the phrase is from The Michigan School Moderator. The earliest known appearance of the phrase is from The Michigan School Moderator. The earliest known appearance of the phrase is from The Michigan School Moderator. The earliest known appearance of the phrase is from The Michigan School Moderator. The earliest known appearance of the phrase is from The Michigan School Moderator. The earliest known appearance of the phrase is from The Michigan School Moderator. The earliest known appearance of the phrase is from The Michigan School Moderator. The earliest known appearance of the phrase is from The Michigan School Moderator. The earliest known appearance of the phrase is from The Michigan School Moderator. The earliest known appearance of the phrase is from The Michigan School Moderator. The earliest known appearance of the phrase is from The Michigan School Moderator. The earliest known appearance of the phrase is from The Michigan School Moderator. The earliest known appearance of the phrase is from The Michigan School Moderator. The earliest known appearance of the phrase is from The Michigan School Moderator. The earliest known appearance of the phrase is from The Michigan School Moderator. The earliest known appearance of the phrase is from The Michigan School Moderator. The earliest known appearance of the phrase is from The Michigan School Moderator. The earliest known appearance of the phrase is from The Michigan School Moderator. The earliest known appearance of the phrase is from The Michigan School Moderator. The earliest known appearance of the phrase is from The Michigan School Moderator. The earliest known appearance of the phrase is from The Michigan School Moderator. The earliest known appearance of the phrase is from The Michigan School Moderator. The earliest known appearance of the phrase is from The Michigan School Moderator. The earliest known appearance of the phrase is from The Michigan School Moderator. The earliest known appearance of the phrase is from The Michigan School Moderator. The earliest known appearance of the phrase is from The Michigan School Moderator. ");

t4.addEventListener("mouseover", function(e){
	this.hover = true;
});

t4.addEventListener("mouseout", function(e){
	this.hover = false;
});


div.addChild(t1);
div.addChild(t2);
div.addChild(t3);
div.addChild(t4);
div.addChild(t5);
div.addChild(t6);
div.addChild(t7);


t2.color = "red";
t4.color = "#ffdd88";
t4.cursor = "pointer";

t4.animate("angle", 0, 720*10, 5000);

t6.fontWeight = "bolder";
t6.color = "grey";
t7.color = "rgba(0, 0, 0, 0.5)";

/*
div.addChild(t7);
div.addChild(t7);
div.addChild(t7);
div.addChild(t7);
div.addChild(t7);
*/


var label = new UILabel(body, {
	label : "The quick brown ",
	left : 250,
	top : 80,
	color : "#ffffff",
	fontSize : 11,
	fontType : "arial"
});


body.add("UIButton").click(function(){
	console.clear();
//	t4.remove();
});


/*
var h = new UIToolTip(t4, {
	label : "Click here to see the Quick Brown Fox"
});
h.enable();
h.show();
h.visible = false;
*/


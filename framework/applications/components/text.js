/* ------------------------+------------- */
/* Native Framework 2.0    | Falcon Build */
/* ------------------------+------------- */
/* (c) 2013 nidium.com - Vincent Fontaine */
/* -------------------------------------- */

/* ---------------------------------------------------------------------------*/
Native.StyleSheet.load("../applications/components/style.nss");
/* ---------------------------------------------------------------------------*/

var body = new Application({
	backgroundImage : "private/assets/black.png",
	class : "body"
});

var div = new UIView(body, {
	left : 251,
	top : 100,
	width : 300,
	height : 250,
	innerWidth : 300,
	fontSize : 14,
	fontFamily : "menlo",
	lineHeight : 22,
	background : "rgba(255, 255, 255, 0.04)",
	color : "#F8F8F2",
	textAlign : "justify",
	borderWidth : 8,
	radius : 7,
	borderColor : "rgba(0, 0, 0, 1)",
	overflow : false,
	scrollable : true,
	class : "div"
});

var extenser = div.add("UIElement", {
	top : 180,
	left : 0,
	width : 300,
	height : 10,
	background : "white",
	opacity : 0
});

//var t0 = new TextNode("The quick brown fox jumps over the lazy dog. The");



var t1 = new TextNode("The quick brown ");
var t2 = new TextNode("fox");
var t3 = new TextNode(" jumps over the lazy ");
var t4 = new TextNode("dog");
var t5 = new TextNode(". ");
var t6 = new TextNode("The earliest known appearance of the phrase");
var t7 = new TextNode(" is from The Michigan ");
var t8 = new TextNode("School");
var t9 = new TextNode(" Moderator. The earliest known appearance of the phrase is from The Michigan School Moderator. ");


div.addChild(t1);
div.addChild(t2);
div.addChild(t3);
div.addChild(t4);
div.addChild(t5);
div.addChild(t6);
div.addChild(t7);
div.addChild(t8);
div.addChild(t9);



t2.color = "red";
t4.color = "#ffdd88";
t4.cursor = "pointer";

t2.animate("angle", 0, 720*10, 5000);
//t8.animate("angle", 0, 720*10, 5000);

t6.fontWeight = "bolder";
t6.color = "grey";
t7.color = "rgba(0, 0, 0, 0.5)";
t8.color = "#4488EE";
t9.color = "grey";





var t100 = new TextNode("getBoundingRect", "method");
var t100a = new TextNode(" = ", "operand");
var t100b = new TextNode("function", "reserved");
var t100c = new TextNode("(){\n");

var t101 = new TextNode("	var ", "reserved");
var t101a = new TextNode("x1");
var t101b = new TextNode(" = ", "operand");
var t101c = new TextNode("this.__left,\n");

var t102 = new TextNode("		y1");
var t102a = new TextNode(" = ", "operand");
var t102b = new TextNode("this.__top,\n");

var t103 = new TextNode("		x2");
var t103a = new TextNode(" = ", "operand");
var t103b = new TextNode("x1");
var t103c = new TextNode(" + ", "operand");
var t103d = new TextNode("this.width,\n");

var t104 = new TextNode("		y2");
var t104a = new TextNode(" = ", "operand");
var t104b = new TextNode("y1");
var t104c = new TextNode(" + ", "operand");
var t104d = new TextNode("this.height,\n");

var t105 = new TextNode("");

var t106 = new TextNode("		origin");
var t106a = new TextNode(" = ", "operand");
var t106b = new TextNode("{\n");

var t107 = new TextNode("			x : this.__left");
var t107a = new TextNode(" + ", "operand");
var t107b = new TextNode("this._width / ");
var t107c = new TextNode("2", "number");
var t107d = new TextNode(",\n");

var t108 = new TextNode("			y : this.__top");
var t108a = new TextNode(" + ", "operand");
var t108b = new TextNode("this._height / ");
var t108c = new TextNode("2\n", "number");

var t109 = new TextNode("		};\n");
var t110 = new TextNode("\n");
var t111 = new TextNode("\n");
var t112 = new TextNode("\n");

var t113 = new TextNode("	if", "operand");
var t113a = new TextNode(" (this.angle");
var t113b = new TextNode(" % ", "operand");
var t113c = new TextNode("360", "number");
var t113d = new TextNode(" === ", "operand");
var t113e = new TextNode("0", "number");
var t113f = new TextNode("){\n");

var t114 = new TextNode("		return", "operand");
var t114a = new TextNode(" {\n");

var t115 = new TextNode("			x1 : x1,\n");
var t116 = new TextNode("			y1 : y1,\n");
var t117 = new TextNode("			x2 : x2,\n");
var t118 = new TextNode("			y2 : y2\n");
var t119 = new TextNode("		}\n");
var t120 = new TextNode("	}\n");
var t121 = new TextNode("};");









var dat = +new Date();

div.addChild(t100);

var dur = (+new Date()) - dat;
console.log(dur, "ms");


div.addChild(t100a);div.addChild(t100b);div.addChild(t100c);

div.addChild(t101);div.addChild(t101a);div.addChild(t101b);div.addChild(t101c);

div.addChild(t102);div.addChild(t102a);div.addChild(t102b);

div.addChild(t103);div.addChild(t103a);div.addChild(t103b);div.addChild(t103c);div.addChild(t103d);

div.addChild(t104);div.addChild(t104a);div.addChild(t104b);div.addChild(t104c);div.addChild(t104d);

div.addChild(t105);

div.addChild(t106);div.addChild(t106a);div.addChild(t106b);

div.addChild(t107);div.addChild(t107a);div.addChild(t107b);div.addChild(t107c);div.addChild(t107d);

div.addChild(t108);div.addChild(t108a);div.addChild(t108b);div.addChild(t108c);

div.addChild(t109);
div.addChild(t110);
//div.addChild(t111);
//div.addChild(t112);

div.addChild(t113);div.addChild(t113a);div.addChild(t113b);div.addChild(t113c);div.addChild(t113d);div.addChild(t113e);div.addChild(t113f);

div.addChild(t114);div.addChild(t114a);

div.addChild(t115);
div.addChild(t116);
div.addChild(t117);
div.addChild(t118);
div.addChild(t119);
div.addChild(t120);
div.addChild(t121);

var dur = (+new Date()) - dat;
console.log(dur, "ms");



/*
div.childNodes.each(function(){
	console.log(this.id, this.className)
	this.updateClassProperties();
	this.redraw();
});
*/

/*

var zz = [];
for (var i=0; i<3; i++){
	zz.push("X");
}
var kk = zz.join("") + " ";

var ta = new TextNode(kk);

//var ta = new TextNode("The earliest known appearance of the phrase is from The Michigan School Moderator. The quick brown fox jumps over the lazy dog. Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files. he above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software. The quick brown fox jumps over the lazy dog. The earliest known appearance of the phrase is from The Michigan School Moderator. The quick brown fox jumps over the lazy dog. Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files. he above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software. The quick brown fox jumps over the lazy dog. The earliest known appearance of the phrase is from The Michigan School Moderator. The quick brown fox jumps over the lazy dog. Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files. he above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software. The quick brown fox jumps over the lazy dog. The earliest known appearance of the phrase is from The Michigan School Moderator. The quick brown fox jumps over the lazy dog. Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files. he above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software. The quick brown fox jumps over the lazy dog. The earliest known appearance of the phrase is from The Michigan School Moderator. The quick brown fox jumps over the lazy dog. Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files. he above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software. The quick brown fox jumps over the lazy dog. The earliest known appearance of the phrase is from The Michigan School Moderator. The quick brown fox jumps over the lazy dog. Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files. he above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software. The quick brown fox jumps over the lazy dog. The earliest known appearance of the phrase is from The Michigan School Moderator. The quick brown fox jumps over the lazy dog. Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files. he above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software. The quick brown fox jumps over the lazy dog. The earliest known appearance of the phrase is from The Michigan School Moderator. The quick brown fox jumps over the lazy dog. Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files. he above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software. The quick brown fox jumps over the lazy dog. The earliest known appearance of the phrase is from The Michigan School Moderator. The quick brown fox jumps over the lazy dog. Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files. he above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software. The quick brown fox jumps over the lazy dog. The earliest known appearance of the phrase is from The Michigan School Moderator. The quick brown fox jumps over the lazy dog. Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files. he above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software. The quick brown fox jumps over the lazy dog. The earliest known appearance of the phrase is from The Michigan School Moderator. The quick brown fox jumps over the lazy dog. Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files. he above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software. The quick brown fox jumps over the lazy dog. ");
var tb = new TextNode("that's it, the earliest known appearance of the phrase.");

t4.addEventListener("mouseover", function(e){
	this.hover = true;
});

t4.addEventListener("mouseout", function(e){
	this.hover = false;
});

*/



//div.addChild(t0);

/*


*/

//div.addChild(ta);
//div.addChild(tb);


/*

var label = new UILabel(body, {
	label : "The quick brown ",
	left : 250,
	top : 80,
	color : "#ffffff",
	fontSize : 11,
	fontFamily : "arial"
});
*/

body.add("UIButton").click(function(){
//	extenser.width = 150;

var dat = +new Date();
	div.maxWidth = 400;
	div.width = 400;

	DOMElement.nodes.refresh(div);

var dur = (+new Date()) - dat;
console.log(dur, "ms");

});


/*
var h = new UIToolTip(t4, {
	label : "Click here to see the Quick Brown Fox"
});
h.enable();
h.show();
h.visible = false;
*/







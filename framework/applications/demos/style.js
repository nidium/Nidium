/* ---------------------------------------------------------------------------*/
/* NSS : Native Style Sheet Demo 1.0                                          */ 
/* ---------------------------------------------------------------------------*/

"use strict";

/* ---------------------------------------------------------------------------*/
load("applications/demos/style.nss");
/* ---------------------------------------------------------------------------*/

Native.StyleSheet.add(style);

var body = new Application();
body.className = "blue";

var label = new UILabel(body, {
	left : 10,
	top : 10,
	paddingLeft : 8,
	paddingRight : 8,
	height : 28,
	color : "#ffffff",
	background : "rgba(255, 255, 255, 0.25)",
	fontSize : 12,
	radius : 4,
	label : "Native Style Sheet Demo"
});

var	bb = new UIButton(body, {
	class : "button doit dark"
});

var	b1 = new UIButton(body, {left:10, class:"button demo blue"}),
	b2 = new UIButton(body, {left:62, class:"button demo blue"}),
	b3 = new UIButton(body, {left:114, class:"button demo blue"});

bb.addEventListener("mousedown", function(e){
	var buttons = Native.layout.getElementsByClassName("blue");
	buttons.each(function(){
		this.removeClass("blue");
		this.addClass("rose");
	});
});

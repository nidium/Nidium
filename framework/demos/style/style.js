/* ------------------------+------------- */
/* Native Framework 2.0    | Falcon Build */
/* ------------------------+------------- */
/* (c) 2013 nidium.com - Vincent Fontaine */
/* -------------------------------------- */

"use strict";

var body = new Application();
body.className = "blue";

var label = new UILabel(body, {
	left : 10,
	top : 10,
	label : "Native Style Sheet Demo",
	class : "label"
});

var	bb = new UIButton(body, "button doit dark");

var	b1 = new UIButton(body, {left:10, class:"button demo blue"}),
	b2 = new UIButton(body, {left:62, class:"button demo blue"}),
	b3 = new UIButton(body, {left:114, class:"button demo blue"});

var t = new UIElement(body, {
	left : 220,
	top : 200,
	width : 16,
	height : 16,
	background : 'black',
	color : '#ffffff'
});

b1.selected = true;
b1.disabled = true;
//b1.applyStyleSheet();

var p = new UIPath(body, {
	left : 250,
	top : 200,
	width : 32,
	height : 32,
	units : 1792,
	advx : 1792,
	advy : 1536,
	path : 'M63 0h-63v1408h63v-1408zM126 1h-32v1407h32v-1407zM220 1h-31v1407h31v-1407zM377 1h-31v1407h31v-1407zM534 1h-62v1407h62v-1407zM660 1h-31v1407h31v-1407zM723 1h-31v1407h31v-1407zM786 1h-31v1407h31v-1407zM943 1h-63v1407h63v-1407zM1100 1h-63v1407h63v-1407z M1226 1h-63v1407h63v-1407zM1352 1h-63v1407h63v-1407zM1446 1h-63v1407h63v-1407zM1635 1h-94v1407h94v-1407zM1698 1h-32v1407h32v-1407zM1792 0h-63v1408h63v-1408z',
	background : 'black',
	color : '#ffffff'
});

bb.addEventListener("mousedown", function(e){
	if (bb.toggle) {
		document.getElementsByClassName("rose").each(function(){
			this.removeClass("rose");
			this.addClass("blue");
		});
		bb.toggle = false;
	} else {
		document.getElementsByClassName("blue").each(function(){
			this.removeClass("blue");
			this.addClass("rose");
		});
		bb.toggle = true;
	}
});
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
	label : "Native Style Sheet Demo",
	class : "label"
});

var	bb = new UIButton(body, {
	class : "button doit dark"
});

var	b1 = new UIButton(body, {left:10, class:"button demo blue"}),
	b2 = new UIButton(body, {left:62, class:"button demo blue"}),
	b3 = new UIButton(body, {left:114, class:"button demo blue"});

bb.addEventListener("mousedown", function(e){
	if (bb.toggle) {
		Native.layout.getElementsByClassName("rose").each(function(){
			this.removeClass("rose");
			this.addClass("blue");
		});
		bb.toggle = false;
	} else {
		Native.layout.getElementsByClassName("blue").each(function(){
			this.removeClass("blue");
			this.addClass("rose");
		});
		bb.toggle = true;
	}
});

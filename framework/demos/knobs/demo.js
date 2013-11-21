/* ------------------------+------------- */
/* Native Framework 2.0    | Falcon Build */
/* ------------------------+------------- */
/* (c) 2013 nidium.com - Vincent Fontaine */
/* -------------------------------------- */

"use strict";

document.backgroundImage = "private://assets/patterns/wood_1.png";

var ox = 80,
	oy = 40;

var k1 = new UIKnob(document, "knob k1");
k1.left = ox;
k1.top = oy+30;

var k2 = new UIKnob(document, "knob k2");
k2.left = ox+64+10;
k2.top = oy+30;

var k3 = new UIKnob(document, "knob k3");
k3.left = ox+128+20;
k3.top = oy+30;

var k4 = new UIKnob(document, "knob k4");
k4.left = ox+192+30;
k4.top = oy+30;

var k5 = new UIKnob(document, "knob k5");
k5.left = 80;
k5.top = 140;
k5.value = -1.0;

var k6 = new UIKnob(document, "knob k6").center();

var k7 = new UIKnob(document, "knob k7");
k7.left = 640;
k7.top = oy+28;

var l1 = new UILabel(document);
l1.left = 230;
l1.top = 140;

var l2 = new UILabel(document);
l2.left = 305;
l2.top = 140;

k1.addEventListener("load", function(){
	this.setValue(100, 4000);
});

k5.addEventListener("load", function(){
	this.setValue(0.0, 800);
});

k3.addEventListener("change", function(e){
	l1.label = Math.round(e.value);
});

k4.addEventListener("change", function(e){
	l2.label = Math.round(100*e.value)/100;
});

k6.addEventListener("change", function(e){
	console.log("k6:", e.value);
});


var power = new UIKnob(document, "power");
power.left = 128;
power.top = 280;
power.click(function(){
	if (this.value==this.min) {
		this.setValue(this.max, 120);
	} else {
		this.setValue(this.min, 120);
	}
});

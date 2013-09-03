/* ------------------------+------------- */
/* Native Framework 2.0    | Falcon Build */
/* ------------------------+------------- */
/* (c) 2013 nidium.com - Vincent Fontaine */
/* -------------------------------------- */

var	button = new UIButton(document, {
	height : 40,
	label : "Click Me",
	color : "rgba(255, 255, 255, 0.55)",
	background : "#8855ff",
	radius : 3
}).center();

button.addEventListener("mouseclick", function(e){
	this.background = "#559933";
	this.label = "Thank You";
	this.paddingLeft = 20;
	this.paddingRight = 20;
	this.center();
});

// a more compact example:

var b = new UIButton().move(10, 10).click(function(e){
	this.fadeOut(350, function(){
		this.remove();
	});
});

document.addChild(b);

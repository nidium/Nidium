/* -------------------------- */
/* Native (@) 2012 Stight.com */
/* -------------------------- */

Native.showFPS(true);

/* --- HELLO APP ------------------------------------------------------------ */

Native.StyleSheet.add({
	button : {
		lineHeight : 14,
		background : "#222222",
		radius : 3,
		fontSize : 12
	},

	blue : {
		background : "#3366bb",
	}
});

var main = new Application({
	backgroundImage : "falcon/assets/back.png",
	background : "#999999"
});

var	b2 = main.add("UIButton", {
	left : 308,
	top : 8,
	label : "Do It"
});

var	ad = new UIView(main, {
	left : 4,
	top : 60,
	height : 300,
	width : 250,
	background : "rgba(50, 50, 50, 0.4)",
	radius : 12
});

var	subad = new UIView(ad, {
	left : 40,
	top : 40,
	height : 80,
	width : 80,
	background : "rgba(250, 50, 50, 0.4)"
});

ad.addEventListener("drag", function(e){
	this.left += e.xrel;
	this.top += e.yrel;
});



b2.addEventListener("mousedown", function(e){
	var elements = Native.layout.getElementsByClassName("button");
	elements.each(function(){
		if (this.hasClass("blue")){
			this.background = "rgb("
				+Math.round(Math.random()*230)+", "
				+Math.round(Math.random()*230)+", "
				+Math.round(Math.random()*230)
			+")";
			this.removeClass("blue");
		} else {
			this.background = "#3366bb";
			this.addClass("blue");
		}
	});
});

b2.background = "#3366bb";





var buttons = [],
	x = 0,
	y = 0;

for (var t=0; t<100; t++){
	buttons[t] = main.add("UIButton", {
		left : 25+x*100,
		top : 350+y*38,
		height : 30,
		lineHeight : 14,
		label : "Button " + (t<10 ? '0' : '') + t,
		background : "rgb("
			+Math.round(Math.random()*230)+", "
			+Math.round(Math.random()*230)+", "
			+Math.round(Math.random()*230)
		+")"
	});

	buttons[t].addClass("button");

	x++;
	if (x>9){
		x = 0;
		y++;
	}

}



/*


b1.addEventListener("change", function(e){
	console.log("b1", e);
});

b2.addEventListener("change", function(e){
	console.log("b2", e);
});


//b1.top = 10;

echo(main.top)

*/
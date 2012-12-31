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
		background : "#3366bb"
	}
});

var main = new Application({
	backgroundImage : "falcon/assets/back.png",
	background : "#999999"
});

var	toggle = new UIButton(main, {
	left : 966,
	top : 8,
	label : "Do It"
});

var	win = new UIView(main, {
	left : 30,
	top : 50,
	width : 608,
	height : 184,
	background : "rgba(50, 50, 150, 0.4)",
	radius : 12
});

win.addEventListener("drag", function(e){
	this.left += e.xrel;
	this.top += e.yrel;
});

toggle.addEventListener("mousedown", function(e){
	var elements = Native.layout.getElementsByClassName("button");
	elements.each(function(){
		if (this.hasClass("blue")){
			this.background = "rgb("
				+Math.round(Math.random()*180)+", "
				+Math.round(Math.random()*180)+", "
				+Math.round(Math.random()*180)
			+")";
			this.removeClass("blue");
		} else {
			this.background = "#3366bb";
			this.addClass("blue");
		}
	});
});

toggle.background = "#3366bb";

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
			+Math.round(Math.random()*180)+", "
			+Math.round(Math.random()*180)+", "
			+Math.round(Math.random()*180)
		+")"
	});

	buttons[t].addClass("button");

	x++;
	if (x>9){
		x = 0;
		y++;
	}
}

var bwin = [],
	x = 0,
	y = 0;

for (var t=0; t<48; t++){
	bwin[t] = win.add("UIButton", {
		left : 10+x*74,
		top : 10+y*28,
		height : 24,
		fontSize : 12,
		lineHeight : 14,
		radius : 6,
		label : "Button " + (t<10 ? '0' : '') + t,
		color : "rgba(255, 255, 255, 0.95)",
		background : "rgb("
			+Math.round(10+Math.random()*100)+", "
			+Math.round(10+Math.random()*80)+", "
			+Math.round(10+Math.random()*50)
		+")"
	});

	bwin[t].addClass("bwin");

	x++;
	if (x>7){
		x = 0;
		y++;
	}

}

var lay = win.add("UIView", {
	left : 630,
	top : 0,
	width : 100,
	height : 100,
	background : "rgba(0, 0, 0, 0.7)"
});

var sublay = lay.add("UIView", {
	left : 10,
	top : 10,
	width : 50,
	height : 50,
	background : "red"
});

lay.addEventListener("drag", function(e){
	this.left += e.xrel;
	this.top += e.yrel;
});



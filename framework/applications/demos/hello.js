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

var	toggle = main.add("UIButton", {
	left : 966,
	top : 8,
	label : "Do It"
});

var	win = new UIView(main, {
	left : 80,
	top : 80,
	height : 200,
	width : 700,
	background : "rgba(50, 50, 150, 0.7)",
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

for (var t=0; t<10; t++){
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

for (var t=0; t<5; t++){
	bwin[t] = win.add("UIButton", {
		left : 0+x*76,
		top : 0+y*28,
		height : 24,
		fontSize : 11,
		lineHeight : 14,
		radius : 6,
		label : "Button " + (t<10 ? '0' : '') + t,
		color : "rgba(0, 0, 0, 0.65)",
		background : "rgb("
			+Math.round(128+Math.random()*120)+", "
			+Math.round(128+Math.random()*120)+", "
			+Math.round(128+Math.random()*120)
		+")"
	});

	bwin[t].addClass("bwin");

	x++;
	if (x>7){
		x = 0;
		y++;
	}

}



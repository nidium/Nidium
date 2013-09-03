/* ------------------------+------------- */
/* Native Framework 2.0    | Falcon Build */
/* ------------------------+------------- */
/* (c) 2013 nidium.com - Vincent Fontaine */
/* -------------------------------------- */

var main = new Application({
	backgroundImage : "images/tunnel.jpeg"
});

var	button = new UIButton(main, {
	id : "button",
	left : 966,
	top : 8,
	label : "Do It"
});

var	win = main.add("UIWindow", {
	left : 60,
	top : 60,
	width : 400,
	height : 300,
	label : 'Main Window',
	background : "rgba(0, 0, 0, 0.4)",
	color : '#ffdddd',
	resizable : true,
	closeable : false,
	movable : true
});

win.width = 320;
win.height = 240;


win.color = "rgba(0, 255, 0, 1)";
win.handle.background = "rgba(255, 0, 0, 0.1)";
win.handle.closeButton.background = "rgba(255, 255, 255, 0.8)";
win.handle.closeButton.color = "rgba(0, 0, 0, 0.7)";
win.contentView.background = "rgba(255, 255, 255, 0.8)";

/* Add 4 children windows to main window */

var w = [];
for (var i=0; i<4; i++){
	w[i] = win.contentView.add("UIWindow", {
		left : i*160,
		top : win.height+20,
		width : 150,
		height : 150,
		background : "rgba(0, 20, 100, 0.5)",
		resizable : true,
		closeable : true,
		movable : true,
		label : 'Child ' + i
	});
	w[i].contentView.background = "rgba(255, 255, 255, 0.90)";
}

w[w.length-1].background = 'rgba(0, 0, 0, 0.2)';
w[w.length-1].contentView.background = 'rgba(0, 0, 0, 0.45)';

var bigview = new Canvas(500, 500);
bigview.padding = 0;
bigview.left = 20;
bigview.top = 20;
bigview.overflow = false;
bigview.context = bigview.getContext("2d");

bigview.context.save();
bigview.context.fillStyle = "#888888";
bigview.context.fillRect(0, 0, 500, 500);
bigview.context.restore();

Native.canvas.add(bigview);

/* --------------- */

var pad = new Canvas(350, 100);
pad.padding = 20;
pad.left = 100;
pad.top = 150;
pad.overflow = true;
pad.context = pad.getContext("2d");
pad.context.save();
pad.context.shadowOffsetX = 0;
pad.context.shadowOffsetY = 0;
pad.context.shadowColor = "rgba(0, 0, 0, 1)";
pad.context.shadowBlur = 8;
pad.context.fillStyle = "yellow";
pad.context.fillRect(0, 0, 350, 100);
pad.context.restore();
bigview.add(pad);


var rectangle = new Canvas(150, 50);
rectangle.padding = 20;
rectangle.left = 0;
rectangle.top = 0;
rectangle.overflow = false;
rectangle.context = rectangle.getContext("2d");
rectangle.context.shadowOffsetX = 0;
rectangle.context.shadowOffsetY = 0;
rectangle.context.shadowColor = "rgba(0, 0, 0, 1)";
rectangle.context.shadowBlur = 8;
pad.add(rectangle);



Canvas.prototype.clear = function(){
	var context = this.getContext("2D");
	context.clearRect(
		-this.padding,
		-this.padding, 
		this.clientWidth,
		this.clientHeight
	);
};



var drag = false;
Native.onmousedown = function(e){
	drag = true;
}
Native.onmouseup = function(e){
	drag = false;
}
Native.canvas.ctx.requestAnimationFrame(function(){
	var w = pad.width,
		h = pad.height;


	rectangle.clear();
	rectangle.left = w - 60;
	rectangle.top = h - 60;
	rectangle.context.fillStyle = "blue";
	rectangle.context.fillRect(0, 0, 150, 50);


	pad.clear();
	pad.context.fillStyle = "yellow";
	pad.context.fillRect(0, 0, w, h);
	pad.context.fillStyle = "rgba(180, 255, 50, 0.08)";
	pad.context.strokeStyle = "rgba(180, 255, 255, 0.4)";
	pad.context.fillRect(
		-pad.padding, 
		-pad.padding, 
		pad.clientWidth,
		pad.clientHeight
	);

	pad.context.strokeRect(
		-pad.padding, 
		-pad.padding, 
		pad.clientWidth,
		pad.clientHeight
	);

});

Native.onmousemove = function(e){
	if (drag) {
		pad.width += e.xrel;
		pad.height += e.yrel;
	}
}



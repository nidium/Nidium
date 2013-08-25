/* ------------------------+------------- */
/* Native Framework 2.0    | Falcon Build */
/* ------------------------+------------- */
/* (c) 2013 Stight.com - Vincent Fontaine */
/* -------------------------------------- */

var bigview = new Canvas(500, 500);
bigview.padding = 0;
bigview.left = 20;
bigview.top = 20;
bigview.ctx.fillStyle = 'black';
bigview.ctx.fillRect(0, 0, 500, 500);
Native.canvas.add(bigview);

/* --------------- */

var context = bigview.getContext("2d"),
	x = 40,
	y = 40,
	w = 100,
	h = 100,
	radius = 60;

	var gradient = context.createRadialGradient(
		x+radius,
		y+radius, 
		radius,
		x+radius,
		y+radius,
		radius/4
	);

	gradient.addColorStop(0.00, 'rgba(255, 255, 255, 0.01)');
	gradient.addColorStop(1.00, 'rgba(255, 255, 255, 1)');

	context.beginPath();
	context.arc(
		x+radius, y+h*0.5, 
		radius, 0, 6.283185307179586, false
	);
	context.setColor("red");
	context.fill();
	context.lineWidth = 1;

	context.beginPath();
	context.arc(
		x+radius, y+h*0.5, 
		radius, 0, 6.283185307179586, false
	);
	context.setColor(gradient);
	context.fill();
	context.lineWidth = 1;

/* ------------------------+------------- */
/* Native Framework 2.0    | Falcon Build */
/* ------------------------+------------- */
/* (c) 2013 nidium.com - Vincent Fontaine */
/* -------------------------------------- */

var bigview = new Canvas(500, 500);
var context = bigview.getContext("2d");
context.fillStyle = 'black';
context.fillRect(0, 0, 500, 500);
window.canvas.add(bigview);

/* --------------- */

var x = 40,
	y = 40,
	w = 100,
	h = 100,
	radius = 50,
	angle = 45;

	var rad = angle * (Math.PI/180),
		origin = {
			x : x + w/2,
			y : y + h/2
		};

	context.save();
	context.translate(origin.x, origin.y);
	context.rotate(rad);
	context.translate(-origin.x, -origin.y);

	context.beginPath();

	context.arc(
		x+radius, y+h*0.5, 
		radius, 0, Math.PI*2, false
	);
	context.fillStyle = "#ff0000";
	context.fill();

	context.fillStyle = "#ffffff";
	context.fillRect(x+w/2, y, 1, h/2);

	context.restore();


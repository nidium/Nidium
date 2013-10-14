/* ------------------------+------------- */
/* Native Framework 2.0    | Falcon Build */
/* ------------------------+------------- */
/* (c) 2013 nidium.com - Vincent Fontaine */
/* -------------------------------------- */

var m = new Canvas(512, 384),
	c1 = m.getContext("2d");

m.padding = 20;
window.canvas.add(m);

c1.fillStyle = "red";
c1.fillRect(0, 0, 1024, 768);

var z = new Canvas(200, 200),
	c2 = z.getContext("2d");

z.padding = 20;
z.left = 0;
z.top = 0;
c2.fillStyle = "blue";
c2.fillRect(0, 0, 200, 200);
m.add(z);


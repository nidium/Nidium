/* ------------------------+------------- */
/* Native Framework 2.0    | Falcon Build */
/* ------------------------+------------- */
/* (c) 2013 nidium.com - Vincent Fontaine */
/* -------------------------------------- */

var m = new Canvas(512, 384);
m.padding = 20;
window.canvas.add(m);

m.ctx.fillStyle = "red";
m.ctx.fillRect(0, 0, 1024, 768);

var z = new Canvas(200, 200);
z.padding = 20;
z.left = 0;
z.top = 0;
z.ctx.fillStyle = "blue";
z.ctx.fillRect(0, 0, 200, 200);
m.add(z);


/* -------------------------- */
/* Native (@) 2012 Stight.com */
/* -------------------------- */

echo("-- client");

var client = new Socket("127.0.0.1", 1053).connect();
client.onconnect = function(){
	echo("-- connected");
};

client.onread = function(data){
	echo("data", data);
};

client.ondisconnect = function(){
	echo("-- disconnected");
};

canvas.onmousemove = function(e){
	client.write(JSON.stringify({
		x : e.x,
		y : e.y
	}));
};

/* ----------------------------------------- */

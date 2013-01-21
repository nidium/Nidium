/* -------------------------- */
/* Native (@) 2012 Stight.com */
/* -------------------------- */

echo("-- server init");

var server = new Socket("127.0.0.1", 1053).listen(),
	mx = 1024/2,
	my = 768/2;

server.onaccept = function(client){
	echo(client);
};

server.onread = function(client, data){
	var e = JSON.parse(data);

	mx = e.x;
	my = e.y;
};

server.ondisconnect = function(client){
	echo("-- disconnected");
};

canvas.fillStyle = "#ffffff";
canvas.requestAnimationFrame(function(){
	canvas.fillRect(mx-1, my-1, 2, 2);
});
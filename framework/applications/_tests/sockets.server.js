/* ------------------------+------------- */
/* Native Framework 2.0    | Falcon Build */
/* ------------------------+------------- */
/* (c) 2013 nidium.com - Vincent Fontaine */
/* -------------------------------------- */

console.log("-- server init");

var server = new Socket("127.0.0.1", 1053).listen(),
	mx = 1024/2,
	my = 768/2;

server.onaccept = function(client){
	console.log(client);
};

server.onread = function(client, data){
	var e = JSON.parse(data);

	mx = e.x;
	my = e.y;
};

server.ondisconnect = function(client){
	console.log("-- disconnected");
};

window.canvas.ctx.fillStyle = "#ffffff";
Native.requestAnimationFrame(function(){
	window.canvas.ctx.fillRect(mx-1, my-1, 2, 2);
});
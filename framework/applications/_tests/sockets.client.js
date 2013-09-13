/* ------------------------+------------- */
/* Native Framework 2.0    | Falcon Build */
/* ------------------------+------------- */
/* (c) 2013 nidium.com - Vincent Fontaine */
/* -------------------------------------- */

console.log("-- client");

var client = new Socket("127.0.0.1", 1053).connect();
client.onconnect = function(){
	console.log("-- connected");
};

client.onread = function(data){
	console.log("data", data);
};

client.ondisconnect = function(){
	console.log("-- disconnected");
};

window._onmousemove = function(e){
	client.write(JSON.stringify({
		x : e.x,
		y : e.y
	}));
};

/* ----------------------------------------- */

/* ------------------------+------------- */
/* Native Framework 2.0    | Falcon Build */
/* ------------------------+------------- */
/* (c) 2013 Stight.com - Vincent Fontaine */
/* -------------------------------------- */

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

window._onmousemove = function(e){
	client.write(JSON.stringify({
		x : e.x,
		y : e.y
	}));
};

/* ----------------------------------------- */

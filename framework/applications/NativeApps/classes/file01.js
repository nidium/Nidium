/* -------------------------- */
/* Native (@) 2013 Stight.com */
/* -------------------------- */

echo("I'm executed first.");

var h1 = new Http("http://www.google.fr").request(function(e){
	echo("request 1 loaded");
});

window.onready = function(){
	echo("document is ready ...");
}

window.onload = function(){
	echo("Every request loaded ...");
};
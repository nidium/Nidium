/* -------------------------- */
/* Native (@) 2013 Stight.com */
/* -------------------------- */

console.log("I'm executed first.");

var h1 = new Http("http://www.google.fr").request(function(e){
	console.log("request 1 loaded");
});

window.onready = function(){
	console.log("document is ready ...");
}

window.onload = function(){
	console.log("Every request loaded ...");
};
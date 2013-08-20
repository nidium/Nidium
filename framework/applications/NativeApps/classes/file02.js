/* -------------------------- */
/* Native (@) 2012 Stight.com */
/* -------------------------- */

var h2 = new Http("http://www.google.fr").request(function(e){
	console.log("request 2 loaded");
});

var m = new Image();
m.src = "assets/bg.png";
m.onload = function(){
	console.log("image loaded");
};

console.log("Hello, I'm file02.js");

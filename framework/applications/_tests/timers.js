/* ------------------------+------------- */
/* Native Framework 2.0    | Falcon Build */
/* ------------------------+------------- */
/* (c) 2013 nidium.com - Vincent Fontaine */
/* -------------------------------------- */

var past = +new Date();

setInterval(function(){
	var now = +new Date();
	console.log("Timer", now-past);
	past = now;
}, 100);


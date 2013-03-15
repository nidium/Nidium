/* ------------------------+------------- */
/* Native Video Player 1.0 | Falcon Build */
/* ------------------------+------------- */
/* (c) 2013 Stight.com - Vincent Fontaine */
/* -------------------------------------- */

var body = new Application({
	backgroundImage : "falcon/assets/black.png",
	class : "body"
});


var video = new UIVideo(body, {
	width : 640,
	height : 360
}).center();

video.load("media/native.mov", function(e){
	this.player.play();
});

video.player.onplay = function(){
	console.log("Start playing ...");
}

video.player.onpause = function(){};
video.player.onstop = function(){};
video.player.onerror = function(e){};



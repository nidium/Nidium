/* ------------------------+------------- */
/* Native Video Player 1.0 | Falcon Build */
/* ------------------------+------------- */
/* (c) 2013 nidium.com - Vincent Fontaine */
/* -------------------------------------- */

var body = new Application({
	backgroundImage : "private://assets/back.png",
	class : "body"
});

var video = new UIVideo(body, {
	width : 640,
	height : 360
}).center();

video.load("../media/bunny.avi", function(e){
	this.player.play();
});

video.player.onplay = function(){
	console.log("Start playing ...");
};

video.player.onpause = function(){};
video.player.onstop = function(){};
video.player.onerror = function(e){};

/*
video.shader("../applications/demos/shaders/radialblur.s", function(program, uniforms){
	var t = 0;
	setInterval(function(){
		uniforms.itime = t++;
	}, 16);
});
*/

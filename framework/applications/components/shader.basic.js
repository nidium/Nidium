/* -------------------------------------- */
/* High Level Shader API (basic demo)     */
/* -------------------------------------- */
/* (c) 2013 nidium.com - Vincent Fontaine */
/* -------------------------------------- */

var main = new Application({
	width : 800,
	height : 600,
	backgroundImage : "applications/components/images/rocks.jpg",
}).center();

main.shader("applications/components/shaders/radialblur.s", function(program, uniforms){
	var t = 0;
	setInterval(function(){
		uniforms.itime = t++;
	}, 16);
});

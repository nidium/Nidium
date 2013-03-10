/* ------------------------+------------- */
/* Native Framework 2.0    | Falcon Build */
/* ------------------------+------------- */
/* (c) 2013 Stight.com - Vincent Fontaine */
/* -------------------------------------- */

var layerPadding = 10;

var main = new Application({
	width : window.width - 2*layerPadding,
	height : window.height - 2*layerPadding,
	backgroundImage : "applications/demos/images/rocks.jpg",
}).center();


var ShaderDemo = {
	pp : 5,

	init : function(){
		var self = this;

		main.shader("applications/demos/shaders/radialblur.s", function(p, u){
			self.start(p, u);
		});
	},

	start : function(program, uniforms){
		var t = 0;

		setInterval(function(){
			uniforms.itime = t++;
		}, 16);
	}
};

ShaderDemo.init();


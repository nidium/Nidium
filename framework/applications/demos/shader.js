/* ------------------------+------------- */
/* Native Framework 2.0    | Falcon Build */
/* ------------------------+------------- */
/* (c) 2013 Stight.com - Vincent Fontaine */
/* -------------------------------------- */

var main = new Application();

var	view = new UIView(main, {
	width : 800,
	height : 600
}).center();

File.getText("applications/demos/shaders/blueplasma.s", function(source){
	var x = 0,
		ctx = view.layer.context,
		program = ctx.attachGLSLFragment(source);
		foo = program.getUniformLocation("itime");

	window.requestAnimationFrame(function(){
		program.uniform1i(foo, ++x);
	});

});


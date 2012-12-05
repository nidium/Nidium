/* -------------------------- */
/* Native (@) 2012 Stight.com */
/* -------------------------- */

/* -- Native FrameWork Demos  ------------------ */

//load("core/native.inc.js");
//load('demos/demo.ui.js');
//load('demos/demo.windows.js');
//load('demos/demo.sliders.js');
//load('demos/demo.diagrams.js');
//load('demos/demo.spectral.js');
//load('demos/demo.tabs.js');

/* -- WebGL Demos --------------------------- */
//load("demos/test.webgl.three.js");
//load("demos/test.webgl.three.particles.js");
load("demos/test.webgl.three.camaro.js");
//load("demos/test.webgl.js");
/* -- Featured Demos --------------------------- */
//load('demos/featured.audiobasics.js');


/* -- Benchmarks ------------------------------- */

//load('bench/suite.js');


/* -- Unit Tests ------------------------------- */

//load('demos/demo.hello.js');
//load('demos/demo.ionBug.js');
//load('demos/demo.timers.js');
//load('demos/demo.http.js');
//load('demos/demo.sockets.js');
//load('demos/demo.flickr.js');

//load('demos/demo.threads.js');
//load('demos/demo.tasks.js');

/* -- StandAlone Demos ------------------------- */

//load('demos/demo.cubewall.js');
//load('demos/demo.box2d.basics.js');
//load('demos/demo.box2d.advanced.js');

//load('demos/demo.smoothie.js');
//load('demos/demo.bluewheel.js');
//load('demos/demo.cube.js');
//load('demos/demo.flamme.js');
//load('demos/demo.particles.js');
//load('demos/demo.raytracer.js');
//load('demos/demo.realtimeRayTracer.js');
//load('demos/demo.sand.js');
//load('demos/demo.water.js');
//load('demos/demo.wires.js');

/*
var background = native.getCanvas2D(1024, 768);

var x = new Canvas(320, 200);
x.zIndex = 5;
x.opacity = 0.5;
x.left = 50;
x.top = 300;
x.hide();

var y = new Canvas(320, 200);
y.zIndex = 6;
y.opacity = 0.9;
y.left = 80;
y.top = 300;

native.requestAnimationFrame = function(){
	if (backgroundModified) redrawBackground();
	for (var i in layer){
		if (layer[i].modified) redrawLayer(1);
	}
};

*/

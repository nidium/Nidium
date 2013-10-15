document.background = "#333333";


//load("applications/_tests/timers.js");
//load("applications/_tests/arc.js");
//load("applications/_tests/canvas.js");
//load("applications/_tests/sockets.client.js");
//load("applications/_tests/sockets.server.js");
//load("applications/_tests/tasks.js");
//load("applications/components/hello.js");
//load("applications/components/motion.js");
//load("applications/components/tabs.js");
//load("applications/components/profiler.js"); // FIXE ME
//load("applications/components/windows.js");
//load("applications/components/dropdown.js");
//load("applications/components/buttons.js");
//load("applications/components/sliders.js");
//load("applications/components/scrollbars.js");
//load("applications/components/modal.js");
load("applications/components/threads.js"); // crash si vidage console
//load("applications/components/tooltips.js");
//load("applications/components/animation.js");
//load("applications/components/flickr.js");
//load("applications/components/http.js");
//load("applications/components/zip.js");

//load("applications/components/text.js"); // FAIL
//load("applications/components/__zzz.js");


/* CANVAS TESTS (SANS NATIVE FRAMEWORK)*/
//load("applications/canvas/water.js"); // fail
//load("applications/canvas/bluewheel.js"); // FAIL (GlobalComposite)
//load("applications/canvas/sand.js"); // OK
//load("applications/canvas/cube.js"); // perf OK
//load("applications/canvas/cubewall.js"); // perf OK
//load("applications/canvas/flamme.js"); // OK
//load("applications/canvas/particles.js"); // perf OK
//load("applications/canvas/text.js"); // FAIL (GlobalComposite)
//load("applications/canvas/shader.js"); // OK

//load("applications/components/flickr.js");


/* TESTS */

    //load("applications/_tests/arc.js");
    //load("testArc.js");
    //load("testRetina.js");


/* MEDIA API */

    //load("applications/audio/test.js"); // OK
    //load("applications/audio/mixer.js"); // OK
    //load("applications/audio/dsp.js"); // OK
    //load("applications/media/video.js"); // OK

/* FILE API */

    //load("applications/components/file.basic.js");
    //load("applications/components/file.advanced.js");


/* SHADERS */

	//load("applications/components/shader.js"); // OK
	//load("applications/components/shader.basic.js"); // OK
	load("applications/components/shader.advanced.js"); // OK


/*

document.nss.add({
	".foobar" : {
		top : 30,
		left : 300,
		width : 450,
		height : 300,
		background : "red"
	},

	".foobar:hover" : {
		background : "#222222",
		radius : 4,
		width : 450,
		height : 80
	},

	".foobar:disabled" : {
		background : "#FF00FF",
		radius : 4,
		width : 450,
		height : 80
	},

	".foobar:disabled+hover" : {
		background : "#0000FF"
	}
});
*/

/*

var txt = new UIButton(document).center();

txt.background = "red";
txt.selected = true;
//txt.className = "foobar";
txt.hover = true;
txt.disabled = true;
*/

/*
var o = new UILabel(document, {
	width : 200,
	autowidth : false,
	paddingLeft : 10,
	paddingRight : 10,
	textAlign : "center"
}).center().move(45, 0);
o.background = "white";
o.label = "Mama's gonna snatch"
*/

/* TUTORIALS */

    //load("applications/tutorials/01.hello.js");
    //load("applications/tutorials/02.buttons.js");
    //load("applications/tutorials/03.events.js");
    //load("applications/tutorials/04.motion.js");
    //load("applications/tutorials/11.post.js");

/* Unfinished */

    //load("applications/components/splines.js"); // LENTEUR ABOMINABLE
    //load("applications/components/diagrams.js");


/* -- Native Debugger ------------------ */

//load("applications/NatBug.nap");



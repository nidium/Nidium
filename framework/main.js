/* ------------------------+------------- */
/* Native Framework 2.0    | Falcon Build */
/* ------------------------+------------- */
/* (c) 2013 Stight.com - Vincent Fontaine */
/* -------------------------------------- */

/* -- Falcon Framework ------------------ */
load("../scripts/preload.js");
load("falcon/native.js");

//load("applications/demos/hello.js");
//load("applications/demos/motion.js");
//load("applications/demos/tabs.js"); // KO ---> arc KO
//load("applications/demos/profiler.js"); // KO
//load("applications/demos/style.js");
//load("applications/demos/windows.js");
//load("applications/demos/dropdown.js");
//load("applications/demos/sliders.js");
//load("applications/demos/scrollbars.js"); // KO : children down
//load("applications/demos/modal.js"); // DOWN
//load("applications/demos/threads.js");
//load("applications/demos/tooltips.js");
//load("applications/demos/animation.js");

/* media demos */
//load("applications/media/video.js");

//load("applications/audio/test.js");
//load("applications/audio/mixer.js");
//load("applications/audio/dsp.js");

/* charts demos */
//load("applications/charts/line.js"); // OK
//load("applications/charts/pie.js"); // KO (pb avec arc)
//load("applications/charts/polar.js"); // KO (pb avec arc)
//load("applications/charts/donut.js"); // KO (pb avec arc)
//load("applications/charts/radar.js"); // OK
//load("applications/charts/bar.js"); // OK
//load("applications/charts/demo.js");


/* Shaders Demos */
//load("applications/demos/shader.js");
//load("applications/demos/shader.basic.js");
//load("applications/demos/shader.advanced.js");

//load("applications/demos/splines.js"); // LENTEUR ABOMINABLE
//load("applications/demos/diagrams.js");

//load("applications/demos/flickr.js");
//load("applications/demos/http.js");

//load("applications/demos/buttons.js");
//load("applications/demos/text.js");
//load("applications/demos/test.js");

//load("testLayer.js"); // KO


/* File API Test */
//load("applications/demos/file.basic.js");
//load("applications/demos/file.advanced.js");

/* CommonJS */
// CommonJS Tests
//load("commonjs/run.js");
// Native module test
/*
var foo = require("testmodule");
console.log("Bar property : " + foo.bar);
foo.hello();
*/


/* -- Native Debugger ------------------ */

//load("applications/NatBug.nap");

/*

var h = new Http("http://p.nf/post.php").request({
    headers: {
        "User-Agent": "firefox",
        "Content-Type": "application/x-www-form-urlencoded",
        "Connection": "close"
    },
    data: "gros=data&foo=bar",
    timeout: 10000
}, function(e) {
    echo(e.data);
});

*/

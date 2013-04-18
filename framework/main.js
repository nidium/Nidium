/* ------------------------+------------- */
/* Native Framework 2.0    | Falcon Build */
/* ------------------------+------------- */
/* (c) 2013 Stight.com - Vincent Fontaine */
/* -------------------------------------- */

/* -- Falcon Framework ------------------ */

require("../scripts/preload.js");
require("falcon/native.js");

//require("applications/demos/hello.js");
//require("applications/demos/motion.js");
//require("applications/demos/tabs.js");
//require("applications/demos/profiler.js");
//require("applications/demos/style.js");
//require("applications/demos/windows.js");
//require("applications/demos/dropdown.js");
//require("applications/demos/sliders.js");
//require("applications/demos/scrollbars.js");
//require("applications/demos/modal.js");
//require("applications/demos/threads.js");
//require("applications/demos/tooltips.js");
//require("applications/demos/animation.js");

/* media demos */
//require("applications/media/video.js");


/* Shaders Demos */
//require("applications/demos/shader.js");
//require("applications/demos/shader.basic.js");
//require("applications/demos/shader.advanced.js");

//require("applications/demos/splines.js");
//require("applications/demos/diagrams.js");

//require("applications/demos/flickr.js");
//require("applications/demos/http.js");

//require("applications/demos/buttons.js");
//require("applications/demos/text.js");
//require("applications/demos/test.js");

//require("testLayer.js");

//require("applications/audio/test.js");
//require("applications/audio/mixer.js");
require("applications/audio/dsp.js");


/* File API Test */
//require("applications/demos/file.basic.js");
//require("applications/demos/file.advanced.js");


/* -- Native Debugger ------------------ */

//require("applications/NatBug.nap");

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

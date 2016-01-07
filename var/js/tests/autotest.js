var COUNTERS = { examples: 0, fails : 0};
try {

test_Canvas_properties_Canvas_scrollTop_0 = function() {
var canvas = new Canvas(200, 400);
var ctx = canvas.getContext("2d");
document.canvas.add(canvas)
ctx.fillStyle = "red";
ctx.fillRect(0, 0, 100, 100);
canvas.top = 100;
document.canvas.offsetTop = 10;
};

test_Canvas_properties_Canvas_position_0 = function() {
var canvas = new Canvas(200, 400);
var ctx = canvas.getContext("2d");
document.canvas.add(canvas);

ctx.fillStyle = "red";
ctx.fillRect(0, 0, 100, 100);

canvas.left = 100; // 100px relative to window.canvas
canvas.top = 50; // 50px relative to window.canvas

// at this stage, canvas is now positioned at (100,50) relative to its parent (window.canvas)

canvas.position = "absolute";

// at this stage, canvas is now positioned at (100,50) relative to the application window
};

test_Canvas_properties_Canvas_top_0 = function() {
var canvas = new Canvas(200, 400);
var ctx = canvas.getContext("2d");
document.canvas.add(canvas);
ctx.fillStyle = "red";
ctx.fillRect(0, 0, 100, 100);
canvas.top = 100;
};

test_Canvas_properties_Canvas_coating_0 = function() {
var canvas = new Canvas(100, 100);
canvas.coating = 100;
canvas.left = 300;
canvas.top = 300;

var ctx = canvas.getContext("2d");
ctx.fillStyle = "red";
ctx.fillRect(0, 0, canvas.width, canvas.height);

ctx.strokeStyle = "blue";
ctx.strokeRect(-100, -100, canvas.width+200, canvas.height+200);

document.canvas.add(canvas);
};

test_Canvas_properties_Canvas_left_0 = function() {
var canvas = new Canvas(200, 400);
var ctx = canvas.getContext("2d");
document.canvas.add(canvas);
ctx.fillStyle = "red";
ctx.fillRect(0, 0, 100, 100);
canvas.left = 100;
};

test_Canvas_properties_Canvas_height_0 = function() {
var canvas = new Canvas(100, 100);
canvas.height = 150;
};

test_Canvas_properties_Canvas_scrollLeft_0 = function() {
var canvas = new Canvas(200, 400);
var ctx = canvas.getContext("2d");
document.canvas.add(canvas)
ctx.fillStyle = "red";
ctx.fillRect(0, 0, 100, 100);
canvas.left = 100;
document.canvas.offsetLeft = 10;
};

test_Canvas_properties_Canvas_clientHeight_0 = function() {
var canvas = new Canvas(100, 100); // create a 100x100 canvas
canvas.coating = 30; // add a coating (padding) of 30 outside the canvas
console.log(canvas.height) // ---> 100 
console.log(canvas.clientHeight) // ---> 160 (30 coating top + 100 height + 30 coating bottom = 160)
};

test_Canvas_properties_Canvas_clientWidth_0 = function() {
var canvas = new Canvas(100, 100); // create a 100x100 canvas
canvas.coating = 30; // add a coating (padding) of 30 outside the canvas
console.log(canvas.width) // ---> 100 
console.log(canvas.clientWidth) // ---> 160 (30 coating left + 100 width + 30 coating right = 160)
};

test_Canvas_properties_Canvas_overflow_0 = function() {
var canvas = new Canvas(100, 100);
document.canvas.add(canvas);
var canvas2 = new Canvas(50, 50);
canvas.add(canvas2);
canvas2.left = 50;
canvas2.top = -10;
canvas.overflow = false;
};

test_Canvas_properties_Canvas_width_0 = function() {
var canvas = new Canvas(100, 100);
canvas.width = 150;
};

test_Canvas_constructors_Canvas_0 = function() {
// Create a new 200x100 canvas (logical pixels)
var myCanvas = new Canvas(200, 100);
// at this stage, myCanvas is an orphaned canvas
// now we add myCanvas to the root canvas (with the add() method)
document.canvas.add(myCanvas); 
// at this stage, myCanvas is "rooted" and document.canvas becomes its parent.
};

test_Canvas_methods_Canvas_getContext_0 = function() {
var context = canvas.getContext("2d");
context.fillStyle = "red";
context.fillRect(0, 0, 200, 100);
};

test_Canvas_methods_Canvas_getPrevSibling_0 = function() {
document.canvas.add(canvas1);
document.canvas.add(canvas2);
document.canvas.add(canvas3);
var prev = canvas2.getPrevSibling(); // returns canvas1
var nothing = canvas1.getPrevSibling(); // returns null
};

test_Canvas_methods_Canvas_removeFromParent_0 = function() {
canvas.removeFromParent();
};

test_Canvas_methods_Canvas_bringToFront_0 = function() {
var canvas = new Canvas(200, 100);
document.canvas.add(canvas);
var canvas2 = new Canvas(10, 10);
document.canvas.add(canvas2);
canvas.bringToFront();
};

test_Canvas_methods_Canvas_clear_0 = function() {
var canvas = new Canvas(200, 400);
var ctx = canvas.getContext("2d");
ctx.fillStyle = "red";
ctx.fillRect(10, 10);
canvas.clear();
};

test_Canvas_methods_Canvas_setCoordinates_0 = function() {
canvas.position = 'absolute';
canvas.setCoordinates(30, 50);
//alternative way
canvas.left = 30;
canvas.top = 50;
};

test_Canvas_methods_Canvas_add_0 = function() {
var canvas = new Canvas(200, 100);
document.canvas.add(canvas);

var canvas2 = new Canvas(10, 10);
canvas.add(canvas2);
};

test_Canvas_methods_Canvas_setZoom_0 = function() {
canvas.zoom(2);
};

test_Canvas_methods_Canvas_setSize_0 = function() {
var canvas = new Canvas(200, 400);
var ctx = canvas.getContext("2d");
canvas.setSize(300, 300);
};

test_Canvas_methods_Canvas_getChildren_0 = function() {
document.canvas.add(canvas1);
document.canvas.add(canvas2);
document.canvas.add(canvas3);
var list = document.canvas.getChildren(); // returns [canvas1, canvas2, canvas3]
canvas2.bringToFront(); // brings canvas2 to the front of the display list.
var list = document.canvas.getChildren(); // returns [canvas1, canvas3, canvas2]
};

test_Canvas_methods_Canvas_getLastChild_0 = function() {
var child = canvas.getLastChild();
};

test_Canvas_methods_Canvas_getFirstChild_0 = function() {
var child = canvas.getFirstChild();
};

test_Canvas_methods_Canvas_getParent_0 = function() {
var parent = canvas.getParent();
};

test_Canvas_methods_Canvas_sendToBack_0 = function() {
var canvas = new Canvas(200, 100);
document.canvas.add(canvas);

var canvas2 = new Canvas(10, 10);
document.canvas.add(canvas2);

canvas2.sendToBack();
};

test_Canvas_methods_Canvas_getNextSibling_0 = function() {
document.canvas.add(canvas1);
document.canvas.add(canvas2);
document.canvas.add(canvas3);
var next = canvas2.getNextSibling(); // returns canvas3
var nothing = canvas3.getNextSibling(); // returns null
};

test_Canvas_base_Canvas_0 = function() {
// Create a new 200x100 canvas (logical pixels)
var canvas = new Canvas(200, 100);
// Add it to the root hierarchy
document.canvas.add(canvas);
// Get its 2D context
var context = canvas.getContext("2d");
};

test_Window_properties_Window___nidium___build_0 = function() {
console.log('build is now: '  + Window.__nidium.build);
};

test_Window_properties_Window___nidium___version_0 = function() {
console.log('version is now: '  + Window.__nidium.version);
};

test_Window_properties_Window_navigator_0 = function() {
console.log(JSON.stringify(Window.navigator));
};

test_Window_properties_Window___nidium___0 = function() {
console.log(JSON.stringify(Window.navigator));
};

test_Window_properties_Window___nidium___revision_0 = function() {
console.log('revision is now: '  + Window.__nidium.revision);
};

test_Audio_properties_Audio_position_0 = function() {
console.log(source.position); // display the current position in the source
source.position += 5; // seek 5 seconds forwards
};

test_Audio_properties_Audio_metadata_0 = function() {
for (var k in source.metadata) {
		console.log("Metadata : " + k + "=" + source.metadata[k]);
	}
};

test_Audio_base_Audio_0 = function() {
// The following snippet demonstrates loading and playing a sound sample:
// * Get the audio context with 'Audio.getContext()'
var dsp = Audio.getContext();

// * Create nodes using the `createNode()` method
var src = dsp.createNode("source", 0, 2);  // No input, 2 output channels
var out = dsp.createNode("target", 2, 0); // 2 input channels, No output

// * Connect the nodes using the 'AudioNode.connect()' method
dsp.connect(src.output[0], out.input[0]);
dsp.connect(src.output[1], out.input[1]);

// * Open a mp3 file
source.open("path/to/file.mp3");

// * And play it
source.play();
};

test_AudioContext_properties_AudioContext_volume_0 = function() {
var dsp = Audio.getContext();
console.log("Volume is " + dsp.volume);
dsp.volume = 0.5;
console.log("Volume is now " + dsp.volume);
};

test_AudioContext_methods_AudioContext_createNode_0 = function() {
var dsp = Audio.getContext();
try {
    var src = dsp.createNode("source", 0, 2);
    var out = dsp.createNode("target", 2, 0);
} catch (e) {
    console.log("Failed to create an audio node", e);
}
};

test_AudioContext_methods_AudioContext_run_0 = function() {
Audio.run(function() {
    // Set foo variable on the JavaScript context of the Audio thread
    this.foo = "bar";
});
};

test_AudioContext_base_AudioContext_0 = function() {
var dsp = Audio.getContext();
var source = dsp.createNode("custom-source", 0, 2);
var target = dsp.createNode("target", 2, 0);

dsp.connect(source.output[0], target.input[0]);
dsp.connect(source.output[1], target.input[1]);

source.process = function(frames, scope) {
    for (var i = 0; i < frames.data.length; i++) {
        for (var j = 0; j < frames.size; j++) {
            var t = j/frames.size;
            frames.data[i][j] = Math.sin(Math.PI * t);
        }
    }
};
source.play();
};

test_Image_base_Image_0 = function() {
document.background = "#333333";
document.opacity = 0;

var pic = new UIElement(document, {
    visible : false,
    width : 1,
    height : 1,
    radius : 6,
    shadowBlur : 6,
    shadowColor : "black"
});

var img = new Image();

img.onload = function(){
    pic.setBackgroundImage(img);
    pic.width = img.width;
    pic.height = img.height;
    pic.visible = true;
    pic.center();
    document.fadeIn(850);
};

img.src = "http://www.nidium.com/static/img/island.png";
};

test_Video_properties_Video_metadata_0 = function() {
for (var k in source.metadata) {
		console.log("Metadata : " + k + "=" + source.metadata[k]);
	}
};

test_Video_properties_Video_position_0 = function() {
console.log(source.position); // display the current position in the source
source.position += 5; // seek 5 seconds forwards
};

test_Navigator_properties_Navigator_appVersion_0 = function() {
console.log('appVersion is now: ' +  window.navigator.appVersion);
};

test_Navigator_properties_Navigator_vibrate_0 = function() {
console.log('vibrate is now: ' +  window.navigator.vibrate);
};

test_Navigator_properties_Navigator_userAgent_0 = function() {
console.log('userAgent is now: ' +  window.navigator.userAgent);
};

test_Navigator_properties_Navigator_platform_0 = function() {
console.log('platform is now: ' +  window.navigator.platform);
};

test_Navigator_properties_Navigator_language_0 = function() {
console.log('language is now: ' +  window.navigator.language);
};

test_Navigator_properties_Navigator_appName_0 = function() {
console.log('appName is now: ' +  window.navigator.appName);
};

test_Navigator_base_Navigator_0 = function() {
console.log(JSON.stringify(Navigator));
};

test_AudioNode_methods_AudioNode_set_0 = function() {
var dsp = Audio.getContext();
var delay = dsp.createNode("delay", 2, 2);
var custom = dsp.createNode("custom", 2, 2);

// Set individual values for a node
delay.set("delay", 500);
delay.set("wet", 0.5);

// You can also pass an object with key/pair to set
delay.set({
    delay: 500,
    wet: 0.5
});

// Custom nodes also support the set() 
// Any type of JS variables can be passed
custom.set("data", {
    name: "Hello world",
    anArray: [13, 37],
    theAnswer: 42
});

// Variables are copied to the audio context, not shared
var foo = 15;
custom.set("foo", foo); 
// Changing local variable does not affect the value of "foo" in the audio context.
foo = 0; 

// A setter is also called everytime a value is set
custom.setter = function(key, value) {
    switch (key) {
        case "foo":
            console.log("foo variable set to ", value);
            break;
    }
};
custom.process = function(frame, scope) {
    // Variable passed to the node using set() are available on the node instance
    this.foo; // 15
    this.data.theAnswer // 42
    this.data.name // Hello world
};
};

test_AudioNode_base_AudioNode_0 = function() {
var dsp = Audio.getContext();
var delay = dsp.createNode("delay", 2, 2);
var custom = dsp.createNode("custom", 2, 2);

// Set individual values for a node
delay.set("delay", 500);
delay.set("wet", 0.5);

// You can also pass an object with key/pair to set
delay.set({
	delay: 500,
    wet: 0.5
});

// Custom nodes also support the set() 
// Any type of JS variables can be passed
custom.set("data", {
    name: "Hello world",
    anArray: [13, 37],
    theAnswer: 42
});

// Variables are copied to the audio context, not shared
var foo = 15;
custom.set("foo", foo); 
// Changing local variable does not affect the value of "foo" in the audio context.
foo = 0; 

// A setter is also called everytime a value is set
custom.setter = function(key, value) {
	switch (key) {
    	case "foo":
            console.log("foo variable set to ", value);
        	break;
    }
};
custom.process = function(frame, scope) {
	// Variable passed to the node using set() are available on the node instance
    this.foo; // 15
    this.data.theAnswer // 42
    this.data.name // Hello world
};
};

test_AudioNode_events_AudioNode_setter_0 = function() {
var dsp = Audio.getContext();
var custom = dsp.createNode("custom", 2, 2);
var foo = 15;
custom.set("foo", foo); 
// Changing local variable does not affect the value of "foo" in the audio context.
foo = 0; 
// A setter is also called everytime a value is set
custom.setter = function(key, value) {
	switch (key) {
    	case "foo":
            console.log("foo variable set to ", value);
        	break;
    }
};
};

test_AudioNode_events_AudioNode_process_0 = function() {
var dsp = Audio.getContext();
var source = dsp.createNode("custom-source", 0, 2);
var target = dsp.createNode("target", 2, 0);

dsp.connect(source.output[0], target.input[0]);
dsp.connect(source.output[1], target.input[1]);
source.process = function(frames, scope) {
    for (var i = 0; i < frames.data.length; i++) {
        for (var j = 0; j < frames.size; j++) {
            var t = j/frames.size;
            frames.data[i][j] = Math.sin(Math.PI * t);
        }
    }
};
};

test_AudioNode_events_AudioNode_process_1 = function() {
//**Sharing data from/to the audio context and the main thread**
var dsp = Audio.getContext();
var source = dsp.createNode("custom-source", 0, 2);
var target = dsp.createNode("target", 2, 0);

dsp.connect(source.output[0], target.input[0]);
dsp.connect(source.output[1], target.input[1]);

source.process = function(frames, scope) {
    for (var i = 0; i < frames.data.length; i++) {
        for (var j = 0; j < frames.size; j++) {
            var t = j/frames.size;
            frames.data[i][j] = Math.sin(Math.PI * t * this.coef);
        }
    }
};

source.init = function(frames, scope) {
    this.coef = 1;
};

source.setter = function(key, value) {
	console.log("Setting " + key + " to " + value);
}

source.onmessage = function(name, data) {
	console.log("received message", name, data);
}

setTimeout(function() {
    source.set("coef", 5);
}, 1500);

source.play();
};

} catch( err ) {
	COUNTERS.fails = 1;
	console.log('Syntax error in example code; Go fix that!' + err.message );
}
if ( ! COUNTERS.fails ) {
	try {
		var fns = ['test_Canvas_properties_Canvas_scrollTop_0', 'test_Canvas_properties_Canvas_position_0', 'test_Canvas_properties_Canvas_top_0', 'test_Canvas_properties_Canvas_coating_0', 'test_Canvas_properties_Canvas_left_0', 'test_Canvas_properties_Canvas_height_0', 'test_Canvas_properties_Canvas_scrollLeft_0', 'test_Canvas_properties_Canvas_clientHeight_0', 'test_Canvas_properties_Canvas_clientWidth_0', 'test_Canvas_properties_Canvas_overflow_0', 'test_Canvas_properties_Canvas_width_0', 'test_Canvas_constructors_Canvas_0', 'test_Canvas_methods_Canvas_getContext_0', 'test_Canvas_methods_Canvas_getPrevSibling_0', 'test_Canvas_methods_Canvas_removeFromParent_0', 'test_Canvas_methods_Canvas_bringToFront_0', 'test_Canvas_methods_Canvas_clear_0', 'test_Canvas_methods_Canvas_setCoordinates_0', 'test_Canvas_methods_Canvas_add_0', 'test_Canvas_methods_Canvas_setZoom_0', 'test_Canvas_methods_Canvas_setSize_0', 'test_Canvas_methods_Canvas_getChildren_0', 'test_Canvas_methods_Canvas_getLastChild_0', 'test_Canvas_methods_Canvas_getFirstChild_0', 'test_Canvas_methods_Canvas_getParent_0', 'test_Canvas_methods_Canvas_sendToBack_0', 'test_Canvas_methods_Canvas_getNextSibling_0', 'test_Canvas_base_Canvas_0', 'test_Window_properties_Window___nidium___build_0', 'test_Window_properties_Window___nidium___version_0', 'test_Window_properties_Window_navigator_0', 'test_Window_properties_Window___nidium___0', 'test_Window_properties_Window___nidium___revision_0', 'test_Audio_properties_Audio_position_0', 'test_Audio_properties_Audio_metadata_0', 'test_Audio_base_Audio_0', 'test_AudioContext_properties_AudioContext_volume_0', 'test_AudioContext_methods_AudioContext_createNode_0', 'test_AudioContext_methods_AudioContext_run_0', 'test_AudioContext_base_AudioContext_0', 'test_Image_base_Image_0', 'test_Video_properties_Video_metadata_0', 'test_Video_properties_Video_position_0', 'test_Navigator_properties_Navigator_appVersion_0', 'test_Navigator_properties_Navigator_vibrate_0', 'test_Navigator_properties_Navigator_userAgent_0', 'test_Navigator_properties_Navigator_platform_0', 'test_Navigator_properties_Navigator_language_0', 'test_Navigator_properties_Navigator_appName_0', 'test_Navigator_base_Navigator_0', 'test_AudioNode_methods_AudioNode_set_0', 'test_AudioNode_base_AudioNode_0', 'test_AudioNode_events_AudioNode_setter_0', 'test_AudioNode_events_AudioNode_process_0', 'test_AudioNode_events_AudioNode_process_1'];
		for (var i in fns ) {
			console.log('running: ' + fns[i] );
			global[fns[i]]();
			COUNTERS.examples++;
		}
	} catch ( err ) {
		console.log( err.message );
		COUNTERS.fails++;
	}
	if ( COUNTERS.fails > 0 ) {
		console.log( COUNTERS.fails + ' examples did not run correctly! Go fix them!' );
	} else {
		console.log( "These " + COUNTERS.examples + " examples seem to be ok!" );
	}
}

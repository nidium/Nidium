# Copyright 2016 Nidium Inc. All rights reserved.
# Use of this source code is governed by a MIT license
# that can be found in the LICENSE file.

from dokumentor import *

NamespaceDoc( "Audio", """Nidium provides a low level audio API, a powerful interface to play and process digital audio signal.

Nidium Audio system was designed from scratch and operates in its own separate thread.

It uses an AudioBuffer for sounds. The API supports loading audio file data in multiple formats, such as mp3, wav, ogg, wma, aac.""",
    SeesDocs( "Audio|AudioContext|_GLOBALAudioThread|AudioNodeLink|AudioNode" ),
    [ExampleDoc( """// The following snippet demonstrates loading and playing a sound sample:
// * Get the audio context with 'Audio.getContext()'
var dsp = Audio.getContext();

// * Create nodes using the `createNode()` method
var src = dsp.createNode("source", 0, 2);  // No input, 2 output channels
var out = dsp.createNode("target", 2, 0); // 2 input channels, No output

// * Connect the nodes using the 'AudioNode.connect()' method
dsp.connect(src.output[0], out.input[0]);
dsp.connect(src.output[1], out.input[1]);

// * Open a mp3 file
src.open("path/to/file.mp3");

// * And play it
src.play();""") ],
    products=["Frontend"]
)

NamespaceDoc( "AudioContext", """The AudioContext represents a set of `AudioNode` linked together along with audio parameters (buffer size, number of channel, sample rate).

An audio context controls both the creation of the nodes it contains and the execution of the audio processing, or decoding. You need to create an AudioContext before you do anything else, as everything happens inside a context.""",
    SeesDocs( "Thread|Audio|Audio|AudioContext|_GLOBALAudioThread|AudioNodeLink|AudioNode" ),
    [ ExampleDoc("""var dsp = Audio.getContext();
var source = dsp.createNode("custom-source", 0, 2);
var target = dsp.createNode("target", 2, 0);

dsp.connect(source.output[0], target.input[0]);
dsp.connect(source.output[1], target.input[1]);

source.assignProcessor(function(frames, scope) {
    for (var i = 0; i < frames.data.length; i++) {
        for (var j = 0; j < frames.size; j++) {
            var t = j/frames.size;
            frames.data[i][j] = Math.sin(Math.PI * t);
        }
    }
});
source.play();
""") ],
    section="Audio",
)

NamespaceDoc( "_GLOBALAudioThread", """The global object for the audio thread. This thread is seperated from the UI/Network thread. This object is shared between all the `AudioNode` you can use it to share variables between the nodes.""",
    SeesDocs( "_GLOBALThread|Audio|AudioContext|AudioNodeLink|AudioNode" ),
    NO_Examples,
    section="Audio"
)

ClassDoc( "AudioNodeLink", "`AudioNodeLink` opaque object. This object is returned by the audio node `input` or `output` array",
    SeesDocs( "Audio|AudioContext|_GLOBALAudioThread|AudioNodeLink|AudioNode" ),
    NO_Examples,
    NO_Inherrits,
    NO_Extends,
    section="Audio"
)

NamespaceDoc( "AudioNode", """AudioNode class.

An `AudioNode` can be either a source a processor or a target.

* A source delivers a stream, which can be decoded and played (mp3, wav, ogg, wma, aac).
* A processor can change the stream of data (gain, reverb, delay, ...).
* A target is the audio system output (speakers)""",
    SeesDocs( "Thread|Audio|AudioContext|_GLOBALAudioThread|AudioNodeLink|AudioNode" ),
    [ExampleDoc("""var dsp = Audio.getContext();
var source  = dsp.createNode("source", 2, 2);   // Source node
var delay   = dsp.createNode("delay", 2, 2);    // Processor node
var custom  = dsp.createNode("custom", 2, 2);   // Processor node
var target  = dsp.createNode("target", 2, 0);   // Target node

// Most processor and custom node allows you to set individual values for a node
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
custom.assignSetter(function(key, value) {
    switch (key) {
        case "foo":
            console.log("foo variable set to ", value);
            break;
    }
});

// Variable passed to the node using set() are available on the node instance
custom.assignProcessor(function(frame, scope) {
    this.foo; // 15
    this.data.theAnswer // 42
    this.data.name // Hello world
}); """) ],
    section="Audio"
)

# Dont think we need to document this
#NamespaceDoc( "AudioNodeThreaded", "AudioNode object exposed in the Audio thread",
#    SeesDocs( "Thread|Audio|Audio|AudioContext|_GLOBALAudioThread|AudioNodeLink|AudioNode" ),
#    NO_Examples
#)

ClassDoc( "Video", "Video playing.",
    [ SeeDoc( "Audio" ) ],
    [ExampleDoc("""// Video are rendered inside a Canvas,
// so first, you need to create a Canvas
var c = new Canvas(640, 360);

// The Canvas must be initialized with a 2d context
c.getContext("2d");

// And added to the root canvas, so we can see it
document.canvas.add(c);

// Then create the video with a reference to the Canvas
var video = new Video(c);

// Open the video file
video.open("big_buck_bunny_480p_h264.mov");

// And play it !
video.play();
""", title="Video playback"), ExampleDoc("""var c = new Canvas(640, 360);
c.getContext("2d");
document.canvas.add(c);

var video = new Video(c);
video.open("big_buck_bunny_480p_h264.mov");

video.on("ready", function() {
    var dsp = Audio.getContext();

    var source = video.getAudioNode();
    if (!source) {
        console.log("Video does not have an audio channel");
        return;
    }

    var target = dsp.createNode("target", 2, 0);

    dsp.connect(source.output[0], target.input[0]);
    dsp.connect(source.output[1], target.input[1]);
});

video.play();""", title="Video playback with sound") ],
    NO_Inherrits,
    NO_Extends,
    products=["Frontend"]
)

for klass in ["AudioNode", "Video"]:
    EventDoc( klass + ".error", "Event fired when the source fails to be opened or if a read error happens while playing the media file.",
            SeesDocs(  klass + ".pause|" + klass + ".play|"+ klass + ".stop|" + klass + ".buffering|" + klass + ".ready|" + klass + ".end"),
            [ExampleDoc("""var dsp = Audio.getContext();
var node = dsp.createNode("source", 0, 2);
node.open("invalid_file.wav");
node.addEventListener("error", function(ev) {
    console.log("Error : " + ev.error + ". Code=" + ev.code);
});
""")],
            [ParamDoc( "event", "Event object",
                ObjectDoc([
                    ( "code", "The error code", "integer" ),
                    ( "error", "Description of the errorcode", "string" )
                ]), NO_Default, IS_Obligated)
            ]
    )

    EventDoc( klass + ".buffering", "Event fired when the source is buffering data.",
            SeesDocs(  klass + ".pause|" + klass + ".play|"+ klass + ".stop|" + klass + ".error|" + klass + ".ready|" + klass + ".end"),
            [ExampleDoc("""var dsp = Audio.getContext();
var node = dsp.createNode("source", 0, 2);
node.open("test.wav");
node.addEventListener("buffering", function(ev) {
    console.log("Buffering : " + ev.bufferedByes + "/" + ev.filesize + " starting at " + ev.startByte);
});
    """)],
            [ ParamDoc( "event", "Event object",
                ObjectDoc([
                    ( "filesize", "The size of the file in bytes", "integer" ),
                    ( "startByte", "The start position of the buffered data.", "integer" ),
                    ( "bufferedBytes", "The number of bytes buffered.", "integer" )
                ]), NO_Default, IS_Obligated),
            ]
    )

    EventDoc( klass + ".ready", "Event fired when the source is opened and ready to be played.",
            SeesDocs(  klass + ".pause|" + klass + ".play|"+ klass + ".stop|" + klass + ".error|" + klass + ".buffering|" + klass + ".end"),
            [ExampleDoc("""var dsp = Audio.getContext();
var node = dsp.createNode("source", 0, 2);
node.open("test.wav");
node.addEventListener("ready", function() {
    console.log("Source is ready to be played");
});
    """)],
            [ParamDoc("dummy", "Empty parameter, set to 'undefined'", 'any', 'null', IS_Obligated)]
    )

    EventDoc( klass + ".pause", "Event fired when the source is put on pause.",
            SeesDocs(klass + ".play|"+ klass + ".stop|" + klass + ".error|" + klass + ".buffering|" + klass + ".ready|" + klass + ".end" ),
            NO_Examples,
            [ParamDoc("dummy", "Empty parameter, set to 'undefined'", 'any', 'null', IS_Obligated)]
    )

    EventDoc( klass + ".play", "Event fired when the source is put on play.",
            SeesDocs(  klass + ".pause|"+ klass + ".stop|" + klass + ".error|" + klass + ".buffering|" + klass + ".ready|" + klass + ".end" ),
            NO_Examples,
            [ParamDoc("dummy", "Empty parameter, set to 'undefined'", 'any', 'null', IS_Obligated)]
    )

    EventDoc( klass + ".end", "Event fired when the source reached the end of the file.",
            SeesDocs(  klass + ".pause|" + klass + ".play|"+ klass + ".stop|" + klass + ".error|" + klass + ".buffering|" + klass + ".ready" ),
            NO_Examples,
            [ParamDoc("dummy", "Empty parameter, set to 'undefined'", 'any', 'null', IS_Obligated)]
    )

FieldDoc( "AudioNode.type", "Node name.",
    SeesDocs( "Audio|AudioNode|AudioNodeLink|AudioNode.type|audioNode.input|AudioNode.output" ),
    NO_Examples,
    IS_Dynamic, IS_Public, IS_Readonly,
    "string",
    NO_Default
)

FieldDoc( "AudioNode.input", "AudioNode Input array.",
    SeesDocs( "Audio|AudioNode|AudioNodeLink|AudioNode.type|AudioNode.output" ),
    NO_Examples,
    IS_Dynamic, IS_Public, IS_Readonly,
    "[AudioNodeLink]",
    NO_Default
)

FieldDoc( "AudioNode.output", "AudioNode Output array.",
    SeesDocs( "Audio|AudioNode|AudioNodeLink|AudioNode.type|audioNode.input" ),
    NO_Examples,
    IS_Dynamic, IS_Public, IS_Readonly,
    "[AudioNodeLink]",
    NO_Default
)

FunctionDoc( "AudioContext.pFFT", "Do a FastFourierTransformation.",
    SeesDocs( "Audio|AudioContext|AudioContext.run|AudioContext.load|AudioContext.createNode|AudioContext.connect|AudioContext.disconnect|AudioContext.pFFT" ),
    NO_Examples,
    IS_Static, IS_Public, IS_Slow,
    [ParamDoc( "xArray", "An array of `length` point", "[float]", NO_Default, IS_Obligated ),
     ParamDoc( "yArray", "An array of `length` point", "[float]", NO_Default, IS_Obligated ),
     ParamDoc( "length", "Length of the transformed axis", "integer", NO_Default, IS_Obligated ),
     ParamDoc( "dir", "Direction (-1 for reverse or +1 for forward)", "integer", NO_Default, IS_Obligated ) ],
    NO_Returns
)


FunctionDoc( "Audio.getContext", """Create or retrieve the current AudioContext.

If an argument is 0 or none arguments are providen, the best values are selected.
> You can only have one `AudioContext`, if you call `getContext` multiple times with different parameters the previous `AudioContext` will become invalid. Calling `getContext` without arguments will return the current `AudioContext`""",
    SeesDocs( "Audio|AudioContext" ),
    NO_Examples,
    IS_Static, IS_Public, IS_Fast,
    [ParamDoc( "buffersize", "Buffer size", "integer", "0", IS_Optional ) ,
     ParamDoc( "channels", "Number of channels to use", "integer", "2", IS_Optional ) ,
     ParamDoc( "sampleRate", "Sample rate", "integer", "44100", IS_Optional ) ],
    ReturnDoc( "AudioContext instance", "AudioContext" )
)

FunctionDoc( "AudioContext.run", "Run a function inside the audio thread",
    SeesDocs( "Audio|AudioContext|AudioContext.run|AudioContext.load|AudioContext.createNode|AudioContext.connect|AudioContext.disconnect|AudioContext.pFFT" ),
    [ExampleDoc("""var dsp = Audio.getContext();
dsp.run(function() {
    // Set foo variable on the JavaScript context of the Audio thread
    this.foo = "bar";
});""" ) ],
    IS_Dynamic, IS_Public, IS_Fast,
    [CallbackDoc( "callback", "Function to execute on the Audio thread.", NO_Params ) ],
    NO_Returns
)

# FunctionDoc( "AudioContext.load", "Not Implemented!",
#     SeesDocs( "Audio|AudioContext|AudioContext.run|AudioContext.load|AudioContext.createNode|AudioContext.connect|AudioContext.disconnect|AudioContext.pFFT" ),
#     NO_Examples,
#     IS_Dynamic, IS_Public, IS_Fast,
#     NO_Params,
#     NO_Returns
# )

FunctionDoc( "AudioContext.createNode", """Create a new `AudioNode` of the specified type.

The following nodes are currently supported :

|| **Type** || **Description** || **Input** || **Output** ||
|| `source` || Play a file or a stream || 0 || 1 or 2 ||
|| `custom-source` || Generate sound with JavaScript || 0 || 1 or 2 ||
|| `custom` || Process sound in JavaScript || up to 32 || up to 32 ||
|| `reverb` || Simulate sound reverberation || up to 32 || up to 32 ||
|| `delay` || Delay a sound || up to 32 || up to 32 ||
|| `gain` || Lower or raise the amplitude of the sound || up to 32 || up to 32 ||
|| `target` || An audio node that represent audio output (computer speaker) || 1 or 2 || 0 ||
""",
    SeesDocs( "Audio|AudioContext|AudioContext.run|AudioContext.createNode|AudioContext.connect|AudioContext.disconnect" ),
    [ExampleDoc("""var dsp = Audio.getContext();
try {
    var src = dsp.createNode("source", 0, 2);
    var out = dsp.createNode("target", 2, 0);
} catch (e) {
    console.log("Failed to create an audio node", e);
}""")],
    IS_Dynamic, IS_Public, IS_Fast,
    [ParamDoc( "type", "Type of node ( 'source'|'target'|'custom-source'|'custom'|'reverb'|'delay'|'gain'|'stereo-enhancer')", "string", NO_Default, IS_Obligated ),
    ParamDoc( "inputChannels", "The number of input channels", "integer", NO_Default, IS_Obligated ),
    ParamDoc( "outputChannels", "The number of output channels", "integer", NO_Default, IS_Obligated ) ],
    ReturnDoc( "A AudioNode of the specified type", "AudioNode" )
)

EventDoc( "AudioNode.message", """Event fired when a custom node or custom-source node is sending a message from the audio thread to the main thread.

As the `AudioContext` has it's own thread, communication between threads must be done via messages.""",
    SeesDocs( "AudioNode.set|AudioNode.assignProcessor|AudioNode.assignInit|AudioNode.assignSetter" ),
    NO_Examples,
    [ParamDoc( "event", "The event object",
        ObjectDoc([
            ( "data", "Data of the message", "any" ),
        ]), NO_Default, IS_Obligated ),
    ]
)

FunctionDoc( "AudioNode.assignSeek", """Assign to the node a function to be called when the `AudioNode.position` is changed on a `custom-source` node.""",
    SeesDocs( "AudioNode.assignProcessor|AudioNode.assignInit|AudioNode.assignSetter|AudioNode.position" ),
    NO_Examples,
    IS_Dynamic, IS_Public, IS_Fast,
    [CallbackDoc( "callback", "Function to execute when the node position is changed", [
        ParamDoc( "position", "The new position in seconds.", "float", NO_Default, IS_Obligated ),
        ParamDoc( "scope", "Global Object of the Audio thread", "_GLOBALAudioThread", NO_Default, IS_Obligated )
    ])]
)

FunctionDoc( "AudioNode.assignProcessor", """Assign to the node a function to be called when the node needs to process audio data.

This can be used to fill the audio buffers of the node to generate sound. This callback runs is another thread than the main nidium UI. If you need to pass Variables you must explicitly set them with `AudioNode.set`.""",
    SeesDocs( "AudioNode.assignProcessor|AudioNode.assignInit|AudioNode.assignSetter|AVNode.message" ),
    [ExampleDoc("""var dsp = Audio.getContext();
var source = dsp.createNode("custom-source", 0, 2);
var target = dsp.createNode("target", 2, 0);

dsp.connect(source.output[0], target.input[0]);
dsp.connect(source.output[1], target.input[1]);
source.assignProcessor(function(frames, scope) {
    for (var i = 0; i < frames.data.length; i++) {
        for (var j = 0; j < frames.size; j++) {
            var t = j/frames.size;
            frames.data[i][j] = Math.sin(Math.PI * t);
        }
    }
});

source.play();"""), ExampleDoc("""//**Sharing data from/to the audio context and the main thread**
var dsp = Audio.getContext();
var source = dsp.createNode("custom-source", 0, 2);
var target = dsp.createNode("target", 2, 0);

dsp.connect(source.output[0], target.input[0]);
dsp.connect(source.output[1], target.input[1]);

source.assignProcessor(function(frames, scope) {
    this.send("processing");
    for (var i = 0; i < frames.data.length; i++) {
        for (var j = 0; j < frames.size; j++) {
            var t = j/frames.size;
            frames.data[i][j] = Math.sin(Math.PI * t * this.coef);
        }
    }
});

source.assignInit(function(frames, scope) {
    this.coef = 1;
});

source.assignSetter(function(key, value) {
    console.log("Setting " + key + " to " + value);
});

source.addEventListener("message", function(ev) {
    console.log("received message", ev.data);
});

setTimeout(function() {
    source.set("coef", 5);
}, 1500);

source.play();
""")],
    IS_Dynamic, IS_Public, IS_Fast,
    [CallbackDoc("processor", "Callback function for generating sound", [
        ParamDoc( "frame", "An object with audio buffers",
            ObjectDoc([
                ( "size", "The size of each audio frame in samples per buffer", "integer" ),
                ( "data", "An array of `Float32Array` with the audio data. Each entry of the array represent a channel of the node", "[Float32Array]" )
            ]), NO_Default, IS_Obligated),
        ParamDoc( "scope", "Global Object of the Audio thread", "_GLOBALAudioThread", NO_Default, IS_Obligated )
    ])]
)

FunctionDoc( "AudioNode.assignInit", "Assign a function to a `custom-source` or `custom` node called when the node is initialized.\n\n> * Nodes are initialized when they are starting to process data.",
    SeesDocs( "AudioNode.assignProcessor|AudioNode.assignInit|AudioNode.assignSetter" ),
    NO_Examples,
    IS_Dynamic, IS_Public, IS_Fast,
    [CallbackDoc( "callback", "Function to execute when the node is initialized", [
            ParamDoc( "scope", "Global Object of the Audio thread", "_GLOBALAudioThread", NO_Default, IS_Obligated )
    ])]
)

FunctionDoc( "AudioNode.assignSetter", """Assign a function to the node called when a variable has been set on the node with `AudioNode.set`.
This is very usefull if you need to do some extra processing on the variable being set.""",
    SeesDocs( "AudioNode.assignProcessor|AudioNode.assignInit|Audio|AudioNode.get|AudioNode.set|AudioNode.assignInit" ),
    [ExampleDoc("""var dsp = Audio.getContext();
var custom = dsp.createNode("custom", 2, 2);

custom.assignSetter(function(key, value) {
    switch (key) {
        case "foo":
        console.log("foo variable set to ", value);
        break;
    }
});

var foo = 15;
custom.set("foo", foo);

// Changing local variable does not affect the value of "foo" in the audio context.
foo = 0;
""")],
    IS_Dynamic, IS_Public, IS_Fast,
    [CallbackDoc( "callback", "Function to execute when a variable is set on the `AudioNode`", [
    ParamDoc( "key", "The key name", "string", NO_Default, IS_Obligated ),
    ParamDoc( "value", "The value that has been set", "any", NO_Default, IS_Obligated ),
        ParamDoc( "scope", "Global Object of the Audio thread", "_GLOBALAudioThread", NO_Default, IS_Obligated )
    ])]
)

FunctionDoc( "AudioContext.connect", "Connect an output channel to an input channel.",
    SeesDocs( "Audio|AudioContext|AudioContext.run|AudioContext.load|AudioContext.createNode|AudioContext.connect|AudioContext.disconnect|AudioContext.pFFT" ),
    NO_Examples,
    IS_Dynamic, IS_Public, IS_Fast,
    [ParamDoc( "output", "Output Channel", "AudioNodeLink", NO_Default, IS_Obligated ),
     ParamDoc( "input", "Input Channel", "AudioNodeLink", NO_Default, IS_Obligated ) ],
    NO_Returns
)

FunctionDoc( "AudioContext.disconnect", """Disconnects two channels.

One channel needs to be an input channel, the other channel needs to be an output channel. The order of the parameters is not important.""",
    SeesDocs( "Audio|AudioContext|AudioContext.run|AudioContext.load|AudioContext.createNode|AudioContext.connect|AudioContext.disconnect|AudioContext.pFFT" ),
    NO_Examples,
    IS_Dynamic, IS_Public, IS_Fast,
    [ParamDoc( "output", "Channel 1", "AudioNodeLink", NO_Default, IS_Obligated ),
     ParamDoc( "input", "Channel 2", "AudioNodeLink", NO_Default, IS_Obligated ) ],
    NO_Returns
)

FunctionDoc( "_GLOBALAudioThread.console", "Expose the console object on the Audio thread",
    [ SeeDoc( "Console" ) ],
    [ExampleDoc("""var dsp = Audio.getContext();
dsp.run(function() {
    console.log("Hello Audio");
});""" ) ],
    IS_Static, IS_Public, IS_Fast,
)

FieldDoc( "AudioContext.bufferSize", "Get the number of samples per buffer for the `AudioContext`.",
    SeesDocs( "Audio|AudioContext|AudioContext.volume|AudioContext.bufferSize|AudioContext.channels|AudioContext.sampleRate" ),
    NO_Examples,
    IS_Dynamic, IS_Public, IS_Readonly,
    "integer",
    NO_Default
)

FieldDoc( "AudioContext.channels", "Get the number of channels for the `AudioContext`.",
    SeesDocs( "Audio|AudioContext|AudioContext.volume|AudioContext.bufferSize|AudioContext.channels|AudioContext.sampleRate" ),
    NO_Examples,
    IS_Dynamic, IS_Public, IS_Readonly,
    "integer",
    NO_Default
)

FieldDoc( "AudioContext.sampleRate", "Get the samplerate for the `AudioContext`.",
    SeesDocs( "Audio|AudioContext|AudioContext.volume|AudioContext.bufferSize|AudioContext.channels|AudioContext.sampleRate" ),
    NO_Examples,
    IS_Dynamic, IS_Public, IS_Readonly,
    "integer",
    NO_Default
)

FieldDoc( "AudioContext.volume", "Set or Get volume on the `AudioContext`.",
    SeesDocs( "Audio|AudioContext|AudioContext.volume|AudioContext.bufferSize|AudioContext.channels|AudioContext.sampleRate" ),
    [ExampleDoc("""var dsp = Audio.getContext();
console.log("Volume is " + dsp.volume);
dsp.volume = 0.5;
console.log("Volume is now " + dsp.volume);""")],
    IS_Dynamic, IS_Public, IS_ReadWrite,
    "integer",
    NO_Default
)

FunctionDoc( "AudioNode.set", """Set a value on an `AudioNode` from the main thread.

For native processor node an error will be thrown if the name of the property is unknown.
If the value has been set correctly, the function that was defined with `assigneSetter` will be called.
Variables passed to custom nodes are *copied* to the audio context. Audio context runs in a different thread.
Variables can then be accessed on the node from inside the audio thread using `this[key]|.""",
    SeesDocs( "Audio|AudioNode.send|AudioNode.get|AudioNode.set|AudioNode.assignSetter|AudioNode.assignInit|AudioNode.assignProcessor" ),
    [ ExampleDoc("""var dsp = Audio.getContext();
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
custom.assignSetter(function(key, value) {
    switch (key) {
        case "foo":
            console.log("foo variable set to ", value);
            break;
    }
});
custom.assignProcessor(function(frame, scope) {
    // Variable passed to the node using set() are available on the node instance
    this.foo; // 15
    this.data.theAnswer // 42
    this.data.name // Hello world
});""") ],
    IS_Dynamic, IS_Public, IS_Fast,
    [ ParamDoc( "key", "Key", "string", NO_Default, IS_Obligated ) ,
     ParamDoc( "value", "Value belonging to Key", "any", NO_Default, IS_Obligated ) ],
    NO_Returns
)

FunctionDoc( "AudioNode.get", "Get a value from a custom AudioNode object.\n> Variables passed to custom nodes are *copied* to the audio context (thread).",
    SeesDocs( "Audio|AudioNode.send|AudioNode.get|AudioNode.set" ),
    NO_Examples,
    IS_Dynamic, IS_Public, IS_Fast,
    [ ParamDoc( "key", "Key to find the value for", "string", NO_Default, IS_Obligated) ],
    ReturnDoc( "The value belonging to 'key'", "any" )
)

FunctionDoc( "AudioNode.send", """Send a message from a `custom-source` node or a `custom` node, to the main thread.

The values are copied and can be accessed with the `message` event.""",
    SeesDocs( "Audio|AudioNode.send|AudioNode.get|AudioNode.set|AudioNode.message" ),
    NO_Examples,
    IS_Dynamic, IS_Public, IS_Fast,
    NO_Params,
    NO_Returns
)

FunctionDoc( "AudioNode.open", """Opens an audio file. You can use any stream supported by Nidium.

Supported Audio Formats: mp3, wav, ogg, wma, aac.
>This method is only available on source node
""",
    SeesDocs( "Audio|AudioNode|AudioNode.open|AudioNode.pause|AudioNode.play|AudioNode.stop" ),
    NO_Examples,
    IS_Dynamic, IS_Public, IS_Fast,
    [ParamDoc( "src", "The file or stream to open", "string", NO_Default, IS_Obligated )],
    NO_Returns,
)

FunctionDoc( "AudioNode.play", "Start playback.\n>This method is only available on source node",
    SeesDocs( "Audio|AudioNode|AudioNode.open|AudioNode.pause|AudioNode.play|AudioNode.stop" ),
    NO_Examples,
    IS_Dynamic, IS_Public, IS_Fast,
    NO_Params,
    NO_Returns,
)

FunctionDoc( "AudioNode.pause", "Pause playback.\n>This method is only available on source node",
    SeesDocs( "Audio|AudioNode|AudioNode.open|AudioNode.pause|AudioNode.play|AudioNode.stop" ),
    NO_Examples,
    IS_Dynamic, IS_Public, IS_Fast,
    NO_Params,
    NO_Returns,
)

FunctionDoc( "AudioNode.stop", "Stop playback.\n>This method is only available on source node",
    SeesDocs( "Audio|AudioNode|AudioNode.open|AudioNode.pause|AudioNode.play|AudioNode.stop" ),
    NO_Examples,
    IS_Dynamic, IS_Public, IS_Fast,
    NO_Params,
    NO_Returns,
)

FunctionDoc( "AudioNode.close", """Close a stream.
This method will free all associated resource with the node. You'll have to call `AudioNode.open` if you want to play a new file
>This method is only available on source node""",
    SeesDocs( "Audio|AudioNode|AudioNode.open|AudioNode.pause|AudioNode.play|AudioNode.stop" ),
    NO_Examples,
    IS_Dynamic, IS_Public, IS_Slow,
    NO_Params,
    NO_Returns,
)

FieldDoc( "Video.width", "The width of the video.",
    SeesDocs("Canvas|Image|Video.height|Video.width|Video.position|Video.duration|Video.metadata|Video.bitrate|Video.frame|Video.canvas" ),
    NO_Examples,
    IS_Dynamic, IS_Public, IS_Readonly,
    "integer",
    NO_Default
)

FieldDoc( "Video.height", "The height of the video.",
    SeesDocs("Canvas|Image|Video.height|Video.width|Video.position|Video.duration|Video.metadata|Video.bitrate|Video.frame|Video.canvas" ),
    NO_Examples,
    IS_Dynamic, IS_Public, IS_Readonly,
    "integer",
    NO_Default
)

EventDoc( "Video.frame", "Event fired when a new video frame is drawn.",
    NO_Sees,
    [ ExampleDoc("""var c = new Canvas(720, 400);
var ctx = c.getContext("2d");
document.canvas.add(c);

var video = new Video(c);
video.open("test.ogg");
video.play();

video.addEventListener("frame", function() {
    console.log("drawing frame");
});
""")],
    [ ParamDoc( "video", "Video instance", "Video", NO_Default, IS_Obligated ) ]
)

ConstructorDoc( "Video", "Create a new Video instance.",
    [ SeeDoc( "Image" ), SeeDoc( "Audio" ) ],
    [ ExampleDoc("""var c = new Canvas(720, 400);
var ctx = c.getContext("2d");
document.canvas.add(c);

var video = new Video(c);
video.open("test.ogg");
video.play();

video.addEventListener("ready", function() {
    var audioNode = video.getAudioNode();
    if (!audioNode) return;

    var dsp = Audio.getContext();
    var target = dsp.createNode("target", 2, 0);

    dsp.connect(audioNode.output[0], target.input[0]);
    dsp.connect(audioNode.output[1], target.input[1]);
});
""")],
    [ParamDoc( "canvas", "The canvas parent. This must be a 2d Canvas", "Canvas", NO_Default, IS_Obligated ) ],
    ReturnDoc( "Video instance", "Video" )
)

FunctionDoc("Video.getAudioNode", """Get the audio node associated to the video.

This method will return `null` if it's called before the `ready` event of the video""",
        [ SeeDoc("Video"), SeeDoc("Audio") ],
        NO_Examples,
    IS_Dynamic, IS_Public, IS_Fast,
        NO_Params,
        ReturnDoc("The audio node instance associated to the video or null if the video does not have any audio stream", "AudioNode", nullable=True)
)

for i in ["Video", "AudioNode" ]:
    if i == "AudioNode":
        more = "\n>This property is only available on `source` node"
        preExample = """var src = new Video(c);
src.open("test.ogg");
        """

    else:
        more = "";
        preExample = """
var dsp = Audio.getContext();
var src = dsp.createNode("source", 0, 2);
src.open("path/to/file.mp3");
        """

    FieldDoc( i +".position", "The current position in the " + i + " in seconds.\n Can also be used to set the position to a certain time." + more,
            SeesDocs( i + ".duration|" + i + ".metadata|" + i + ".bitrate" ),
            [ ExampleDoc(preExample + """console.log(src.position); // display the current position in the source
src.position += 5; // seek 5 seconds forwards""")],
            IS_Dynamic, IS_Public, IS_ReadWrite,
            "integer",
            NO_Default
    )

    FieldDoc( i + ".duration", "The duration of the " + i + " in seconds." + more,
            SeesDocs( i + ".position|" + i + ".metadata|" + i + ".bitrate" ),
            NO_Examples,
            IS_Dynamic, IS_Public, IS_Readonly,
            "integer",
            NO_Default
    )

    FieldDoc( i +".bitrate", "The bitrate of the " + i + " in samples per buffer." + more,
            SeesDocs( i + ".position|" + i + ".metadata|" + i + ".duration" ),
            NO_Examples,
            IS_Dynamic, IS_Public, IS_Readonly,
            "integer",
            NO_Default
    )

    FieldDoc( i + ".metadata", "An object with metadata that belong to this " + i + " stream." + more,
            SeesDocs(i + "|" + i + ".position|" + i + ".duration|" + i + ".metadata|" + i + ".bitrate" ),
            [ExampleDoc( preExample + """for (var k in src.metadata) {
    console.log("Metadata : " + k + "=" + src.metadata[k]);
}"""
            )],
        IS_Dynamic, IS_Public, IS_Readonly,
        "Object",
        NO_Default
    )

from dokumentor import *

NamespaceDoc( "ThreadMessageEvent", "Communication channel between threads.",
	SeesDocs( "Threads|Audio" ),
	NO_Examples
)

NamespaceDoc( "Audio", """Nidium provides a low level audio API called NativeDSP, a powerful interface to play and process digital audio signal.

NativeDSP was designed from scratch and operates in its own separate thread.

NativeDSP uses an AudioBuffer for sounds. The API supports loading audio file data in multiple formats, such as mp3, wav, ogg, wma, aac.""",
	SeesDocs( "Thread|Audio|Audio|AudioContext|_GLOBALAudioThread|AudioNodeLink|AudioNodeEvent|AudoNode" ),
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
source.open("path/to/file.mp3");

// * And play it
source.play();""") ]
)

NamespaceDoc( "AudioContext", "The context/thread where the audio originated.",
	SeesDocs( "Thread|Audio|Audio|AudioContext|_GLOBALAudioThread|AudioNodeLink|AudioNodeEvent|AudoNode" ),
	[ ExampleDoc("""var dsp = Audio.getContext();
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
""") ]
)

NamespaceDoc( "_GLOBALAudioThread", """The main thread that is the parent of all the audio threads.
This is seperate from the UI/Network thread.""",
	SeesDocs( "_GLOBALThread|Thread|Audio|Audio|AudioContext|_GLOBALAudioThread|AudioNodeLink|AudioNodeEvent|AudoNode" ),
	NO_Examples
)

ClassDoc( "AudioNodeLink", "AudioNodeLink class.",
	SeesDocs( "Thread|Audio|Audio|AudioContext|_GLOBALAudioThread|AudioNodeLink|AudioNodeEvent|AudoNode" ),
	NO_Examples,
	NO_Inherrits,
	NO_Extends
)

NamespaceDoc( "AudioNodeEvent", "Object that will be used as argument in the various Audio callback functions.",
	SeesDocs( "Thread|Audio|Audio|AudioContext|_GLOBALAudioThread|AudioNodeLink|AudioNodeEvent|AudoNode" ),
	NO_Examples
)

NamespaceDoc( "AudioNode", """AudioNode class

An audionode can be either a source or a target

A source delivers a ( content of a file as a ) stream, which can be decoded and played (mp3, wav, ogg, wma, aac).
A target receives this. A target could be an audio system (speakers) or another audionode.""",
	SeesDocs( "Thread|Audio|Audio|AudioContext|_GLOBALAudioThread|AudioNodeLink|AudioNodeEvent|AudoNode" ),
	[ExampleDoc("""var dsp = Audio.getContext();
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
}; """) ]
)

NamespaceDoc( "AudioNodeThreaded", "An threaded Audio node.",
	SeesDocs( "Thread|Audio|Audio|AudioContext|_GLOBALAudioThread|AudioNodeLink|AudioNodeEvent|AudoNode" ),
	NO_Examples
)

ClassDoc( "Video", "Video playing.",
	[ SeeDoc( "Image" ), SeeDoc( "Audio" ) ],
	NO_Examples,
	NO_Inherrits,
	NO_Extends
)

EventDoc( "ThreadMessageEvent.onmessage", "A message has been send.",
	SeesDocs( "ThreadMessageEvent|ThreadMessageEvent.data|ThreadMessageEvent.onmessage|Thread|Audio" ),
	NO_Examples,
	[ ParamDoc( "e", "error object with key: data", "Object", IS_Obligated ) ]
)

FieldDoc( "ThreadMessageEvent.data", "Data object.",
	SeesDocs( "ThreadMessageEvent|ThreadMessageEvent.data|ThreadMessageEvent.onmessage|Thread|Audio" ),
	NO_Examples,
	IS_Dynamic, IS_Public, IS_Readonly,
	"Object",
	NO_Default
)

EventDoc( "ThreadMessageEvent.onerror", "Callback fired when the source fails to be opened or if a read error happens while playing the song.",
	SeesDocs( "ThreadMessageEvent|ThreadMessageEvent.onpause|ThreadMessageEvent.onplay|ThreadMessageEvent.onstop|ThreadEventMessage.onerror|ThreadEventMessage.onbuffering|ThreadEventMessage.onready" ),
	NO_Examples,
	[ ParamDoc( "error", "The error code", "integer", 'null', IS_Obligated ),
	 ParamDoc( "error", "Description of the errorcode", "string", IS_Obligated )]
)

EventDoc( "ThreadMessageEvent.onbuffering", "Callback fired when the source is buffering data.",
	SeesDocs( "ThreadMessageEvent|ThreadMessageEvent.onpause|ThreadMessageEvent.onplay|ThreadMessageEvent.onstop|ThreadEventMessage.onerror|ThreadEventMessage.onbuffering|ThreadEventMessage.onready" ),
	NO_Examples,
	[ ParamDoc( "size", "The size of the file in bytes", "integer", NO_Default, IS_Obligated ),
	 ParamDoc( "start", "The start position of the buffered data.", NO_Default, IS_Obligated ),
	 ParamDoc( "end", "The end position of the buffered data.", NO_Default, IS_Obligated ) ]
)

EventDoc( "ThreadMessageEvent.onready", "Callback fired when the source is opened and ready to play.",
	SeesDocs( "ThreadMessageEvent|ThreadMessageEvent.onpause|ThreadMessageEvent.onplay|ThreadMessageEvent.onstop|ThreadEventMessage.onerror|ThreadEventMessage.onbuffering|ThreadEventMessage.onready" ),
	NO_Examples,
	NO_Params
)

EventDoc( "ThreadMessageEvent.onpause", "Callback fired when the source is put on pause.",
	SeesDocs( "ThreadMessageEvent|ThreadMessageEvent.onpause|ThreadMessageEvent.onplay|ThreadMessageEvent.onstop|ThreadEventMessage.onerror|ThreadEventMessage.onbuffering|ThreadEventMessage.onready" ),
	NO_Examples,
	NO_Params
)

EventDoc( "ThreadMessageEvent.onplay", "Callback fired when the source is put on play.",
	SeesDocs( "ThreadMessageEvent|ThreadMessageEvent.onpause|ThreadMessageEvent.onplay|ThreadMessageEvent.onstop|ThreadEventMessage.onerror|ThreadEventMessage.onbuffering|ThreadEventMessage.onready" ),
	NO_Examples,
	NO_Params
)

FieldDoc( "AudioNode.type", "Node name.",
	SeesDocs( "Audio|AudioNode|AudioNodeLink|AudioNode.type|audioNode.input|AudioNode.output" ),
	NO_Examples,
	IS_Dynamic, IS_Public, IS_Readonly,
	"AudioNodeLink",
	NO_Default
)

FieldDoc( "AudioNode.input", "AudioNode Input array.",
	SeesDocs( "Audio|AudioNode|AudioNodeLink|AudioNode.type|audioNode.input|AudioNode.output" ),
	NO_Examples,
	IS_Dynamic, IS_Public, IS_Readonly,
	"array",
	NO_Default
)

FieldDoc( "AudioNode.output", "AudioNode Output array.",
	SeesDocs( "Audio|AudioNode|AudioNodeLink|AudioNode.type|audioNode.input|AudioNode.output" ),
	NO_Examples,
	IS_Dynamic, IS_Public, IS_Readonly,
	"array",
	NO_Default
)

FieldDoc( "AudioNodeEvent.data", "Data frames that are ready for further processing.",
	[ SeeDoc( "AudioNodeEvent.size" ), SeeDoc( "AudioNodeEvent.size" ) ],
	NO_Examples,
	IS_Dynamic, IS_Public, IS_Readonly,
	'object',
	NO_Default
)

FieldDoc( "AudioNodeEvent.size", "Size of th data frames.",
	[ SeeDoc( "AudioNodeEvent.size" ), SeeDoc( "AudioNodeEvent.size" ) ],
	NO_Examples,
	IS_Dynamic, IS_Public, IS_Readonly,
	'integer',
	NO_Default
)

FunctionDoc( "AudioContext.pFFT", "Do a FastFourierTransformation.",
	SeesDocs( "Audio|AudioContext|AudioContext.run|AudioContext.load|AudioContext.createNode|AudioContext.connect|AudioContext.disconnect|AudioContext.pFFT" ),
	NO_Examples,
	IS_Dynamic, IS_Public, IS_Slow,
	[ParamDoc( "x", "X,", "[float]", NO_Default, IS_Obligated ),
	 ParamDoc( "y", "Y", "[float]", NO_Default, IS_Obligated ),
	 ParamDoc( "n", "N", "integer", NO_Default, IS_Obligated ),
	 ParamDoc( "dir", "Direction", "integer", NO_Default, IS_Obligated ) ],
	NO_Returns
)


FunctionDoc( "Audio.getContext", """Construct a new AudioContext.

If an argument is 0 or none arguments are providen, the best values are selected.""",
	SeesDocs( "Audio|AudioContext" ),
	NO_Examples,
	IS_Static, IS_Public, IS_Fast,
	[ParamDoc( "buffersize", "Buffer size", "integer", "2048", IS_Optional ) ,
	 ParamDoc( "channels", "Number of channels to use", "integer", "2", IS_Optional ) ,
	 ParamDoc( "sampleRate", "Sample rate", "integer", "44100", IS_Optional ) ],
	ReturnDoc( "AudioContext instance", "AudioContext" )
)

FunctionDoc( "AudioContext.run", "Start the audio.",
	SeesDocs( "Audio|AudioContext|AudioContext.run|AudioContext.load|AudioContext.createNode|AudioContext.connect|AudioContext.disconnect|AudioContext.pFFT" ),
	[ExampleDoc("""var dsp = Audio.getContext();
dsp.run(function() {
	// Set foo variable on the JavaScript context of the Audio thread
	this.foo = "bar";
});""" ) ],
	IS_Dynamic, IS_Public, IS_Fast,
	[CallbackDoc( "callback", "Function to execute on the Audio thread when the thread is ready to run.",
		[ ParamDoc ( "?", "?", "Object", NO_Default, IS_Obligated ) ] ) ],
	NO_Returns
)
FunctionDoc( "AudioContext.load", "Not Implemented!",
	SeesDocs( "Audio|AudioContext|AudioContext.run|AudioContext.load|AudioContext.createNode|AudioContext.connect|AudioContext.disconnect|AudioContext.pFFT" ),
	NO_Examples,
	IS_Dynamic, IS_Public, IS_Fast,
	NO_Params,
	NO_Returns
)
FunctionDoc( "AudioContext.createNode", """Create a new 'AudioNode' of the specified type.

The following nodes are currently supported :

|| 'type' || 'Description' || 'Input' || 'Output' ||
|| source || Play a file or a stream || 0 || 1 or 2 ||
|| custom-source || Generate sound with JavaScript || 0 || 1 or 2 ||
|| custom || Process sound in JavaScript || up to 32 || up to 32 ||
|| reverb || Simulate sound reverberation || up to 32 || up to 32 ||
|| delay || Delay a sound || up to 32 || up to 32 ||
|| gain || Lower or raise the amplitude of the sound || up to 32 || up to 32 ||
|| target || An audio node that represent audio output (computer speaker) || 1 or 2 || 0 ||
""",
	SeesDocs( "Audio|AudioContext|AudioContext.run|AudioContext.load|AudioContext.createNode|AudioContext.connect|AudioContext.disconnect|AudioContext.pFFT" ),
	[ExampleDoc("""var dsp = Audio.getContext();
try {
	var src = dsp.createNode("source", 0, 2);
	var out = dsp.createNode("target", 2, 0);
} catch (e) {
	console.log("Failed to create an audio node", e);
}""")],
	IS_Dynamic, IS_Public, IS_Fast,
	[ParamDoc( "type", "Type of node ( 'source'|'target'|'custom-source'|'custom'|'reverb'|'delay'|'gain'|'stereo-enhancer')", "string", NO_Default, IS_Obligated ),
	ParamDoc( "inputchannels", "The number of input channels", "integer", NO_Default, IS_Obligated ),
	ParamDoc( "outputchannels", "The number of output channels", "integer", NO_Default, IS_Obligated ) ],
	ReturnDoc( "A AudioNode of the specified type", "AudioNode" )
)

EventDoc( "AudioNode.onmessage", """Callback of message events.

As every 'AudioContext' has it's own thread, communitation between threads is done via messages.
This event will be fired when an message is send from an 'AudioNode' (via 'AudioNode.set') to the main thread.""",
	SeesDocs( "Thread|AudioNode.process|AudioNode.init|AudioNode.setter" ),
	NO_Examples,
	[ParamDoc( "name", "The name of the message", "string", NO_Default, IS_Obligated ),
	ParamDoc( "data", "Data of the message", "mixed", NO_Default, IS_Obligated ) ],
)

EventDoc( "AudioNode.seek", """Callback when the 'AudioNode.position' is changed on a custom 'AudioNode'.""",
	SeesDocs( "Thread|AudioNode.process|AudioNode.init|AudioNode.setter" ),
	NO_Examples,
	[ParamDoc( "postition", "The new position in the audiostream.", "integer", NO_Default, IS_Obligated ) ],
)

EventDoc( "AudioNode.process", """Callback of process events.

This can be used to fill the audiobuffers of the nodes.
This callback runs is another thread then the main nidium UI. Variables must bes set expicitly with 'AudioNode.set'.""",
	SeesDocs( "AudioNode.process|AudioNode.init|AudioNode.setter" ),
	[ExampleDoc("""var dsp = Audio.getContext();
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
};"""), ExampleDoc("""//**Sharing data from/to the audio context and the main thread**
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
""")],
	[ParamDoc( "frame", "An object with audio buffers", "[AudioBuffer]", NO_Default, IS_Obligated ),
	ParamDoc( "scope", "Javascript context of the Audio thread", "AudioContext", NO_Default, IS_Obligated ) ]
)

EventDoc( "AudioNode.init", "Callback of init events.",
	SeesDocs( "AudioNode.process|AudioNode.init|AudioNode.setter" ),
	NO_Examples,
	[ParamDoc( "scope", "Javascript context of the audio thread.", "AudioContext", NO_Default, IS_Obligated ) ]
)

EventDoc( "AudioNode.setter", """Callback of 'AudioNode.set' events.
A value has been set for a certain key on this thread.
This is very usefull if you need to do some extra processing on the variable being set.""",
	SeesDocs( "AudioNode.process|AudioNode.init|AudioNode.setter|Audio|AudioNode.get|AudioNode.set|AudioNode.setter|AudioNode.init|AudioNode.process" ),
	[ExampleDoc("""var dsp = Audio.getContext();
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
""")],
	[ParamDoc( "key", "Key", "string", NO_Default, IS_Obligated ),
	ParamDoc( "value", "the value that has been set", "mixed", NO_Default, IS_Obligated ),
	ParamDoc( "global", "Global object for it's thread", "Object", NO_Default, IS_Obligated ) ]
)

FunctionDoc( "AudioContext.connect", "Connect an output channel to an input channel.",
	SeesDocs( "Audio|AudioContext|AudioContext.run|AudioContext.load|AudioContext.createNode|AudioContext.connect|AudioContext.disconnect|AudioContext.pFFT" ),
	NO_Examples,
	IS_Dynamic, IS_Public, IS_Fast,
	[ParamDoc( "output", "Output Channel", "AudioNodeLink", NO_Default, IS_Obligated ),
	 ParamDoc( "input", "Input Channel", "AudioNodeLink", NO_Default, IS_Obligated ) ],
	NO_Returns
)

FunctionDoc( "AudioContext.disconnect", """Disconnects two cannels.

One channel needs to be an input channel, the other channel needs to be an output channel. The order of the parameters is not important.""",
	SeesDocs( "Audio|AudioContext|AudioContext.run|AudioContext.load|AudioContext.createNode|AudioContext.connect|AudioContext.disconnect|AudioContext.pFFT" ),
	NO_Examples,
	IS_Dynamic, IS_Public, IS_Fast,
	[ParamDoc( "output", "Channel 1", "AudioNodeLink", NO_Default, IS_Obligated ),
	 ParamDoc( "input", "Channel 2", "AudioNodeLink", NO_Default, IS_Obligated ) ],
	NO_Returns
)

FunctionDoc( "_GLOBALAudioThread.echo", "Display a textline on the console.",
	[ SeeDoc( "Console" ) ],
	NO_Examples,
	IS_Dynamic, IS_Public, IS_Fast,
	[ ParamDoc( "text", "Text to print", "string", NO_Default, IS_Obligated ) ],
	NO_Returns
)

FieldDoc( "AudioContext.bufferSize", "Get the buffersize for the 'AudioContext' buffer in samples per buffer.",
	SeesDocs( "Audio|AudioContext|AudioContext.volume|AudioContext.bufferSize|AudioContext.channels|AudioContext.sampleRate" ),
	NO_Examples,
	IS_Dynamic, IS_Public, IS_Readonly,
	"integer",
	NO_Default
)

FieldDoc( "AudioContext.channels", "Get the number of channels for the 'AudioContext'.",
	SeesDocs( "Audio|AudioContext|AudioContext.volume|AudioContext.bufferSize|AudioContext.channels|AudioContext.sampleRate" ),
	NO_Examples,
	IS_Dynamic, IS_Public, IS_Readonly,
	"integer",
	NO_Default
)

FieldDoc( "AudioContext.sampleRate", "Get the samplerate for the 'AudioContext'.",
	SeesDocs( "Audio|AudioContext|AudioContext.volume|AudioContext.bufferSize|AudioContext.channels|AudioContext.sampleRate" ),
	NO_Examples,
	IS_Dynamic, IS_Public, IS_Readonly,
	"integer",
	NO_Default
)

FieldDoc( "AudioContext.volume", "Set or Get volume on the 'AudioContext'.",
	SeesDocs( "Audio|AudioContext|AudioContext.volume|AudioContext.bufferSize|AudioContext.channels|AudioContext.sampleRate" ),
	[ExampleDoc("""var dsp = Audio.getContext();
console.log("Volume is " + dsp.volume);
dsp.volume = 0.5;
console.log("Volume is now " + dsp.volume);""")],
	IS_Dynamic, IS_Public, IS_ReadWrite,
	"integer",
	NO_Default
)

FunctionDoc( "AudioNode.set", """Set a value on a AudioNode object from the main thread.

An error will thrown if the name of the property is unknown.
If the value has been set correctly, the function that was defined as 'setter' will be called.
Variables passed to custom nodes are *copied* to the audio context. Every audio context runs in a different thread.
Variables can then be accesed on the node from inside the audio thread using 'this.[key]'.""",
	SeesDocs( "Audio|AudioNode.send|AudioNode.get|AudioNode.set|AudioNode.setter|AudioNode.init|AudioNode.process" ),
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
};""") ],
	IS_Dynamic, IS_Public, IS_Fast,
	[ ParamDoc( "key", "Key to find the value for at a later stage", "string", NO_Default, IS_Obligated ) ,
	 ParamDoc( "value", "Value belonging to Key. All javascript types are accepted.", "mixed", NO_Default, IS_Obligated ) ],
	NO_Returns
)

FunctionDoc( "AudioNode.get", "Get a value from a custom AudioNode object. > Variables passed to custom nodes are *copied* to the audio context (thread).",
	SeesDocs( "Audio|AudioNode.send|AudioNode.get|AudioNode.set" ),
	NO_Examples,
	IS_Dynamic, IS_Public, IS_Fast,
	[ ParamDoc( "key", "Key to find the value for", "string", NO_Default, IS_Obligated) ],
	ReturnDoc( "The value belonging to 'key'", "mixed" )
)

FunctionDoc( "AudioNode.send", """Send, from a AudioNode, a signal to the main hread.

The values are copied and can be accessed with the 'AudioNode.onmessage' event.""",
	SeesDocs( "Audio|AudioNode.send|AudioNode.get|AudioNode.set" ),
	NO_Examples,
	IS_Dynamic, IS_Public, IS_Fast,
	NO_Params,
	NO_Returns
)

FunctionDoc( "AudioNode.open", """Opens an audio file. You can use any stream supported by Nidium.

Supported Audio Formats: mp3, wav, ogg, wma, aac.""",
	SeesDocs( "Audio|AudioNode|AudioNode.open|AudioNode.pause|AudioNode.play|AudioNode.stop" ),
	NO_Examples,
	IS_Dynamic, IS_Public, IS_Fast,
	[ParamDoc( "src", "The file or stream to open", "string", NO_Default, IS_Obligated )],
	NO_Returns,
)

FunctionDoc( "AudioNode.play", "Start playback.",
	SeesDocs( "Audio|AudioNode|AudioNode.open|AudioNode.pause|AudioNode.play|AudioNode.stop" ),
	NO_Examples,
	IS_Dynamic, IS_Public, IS_Fast,
	NO_Params,
	NO_Returns,
)

FunctionDoc( "AudioNode.pause", "Pause playback.",
	SeesDocs( "Audio|AudioNode|AudioNode.open|AudioNode.pause|AudioNode.play|AudioNode.stop" ),
	NO_Examples,
	IS_Dynamic, IS_Public, IS_Fast,
	NO_Params,
	NO_Returns,
)

FunctionDoc( "AudioNode.stop", "Stop playback.",
	SeesDocs( "Audio|AudioNode|AudioNode.open|AudioNode.pause|AudioNode.play|AudioNode.stop" ),
	NO_Examples,
	IS_Dynamic, IS_Public, IS_Fast,
	NO_Params,
	NO_Returns,
)

FunctionDoc( "AudioNode.close", "Close a stream.",
	SeesDocs( "Audio|AudioNode|AudioNode.open|AudioNode.pause|AudioNode.play|AudioNode.stop" ),
	NO_Examples,
	IS_Dynamic, IS_Public, IS_Fast,
	NO_Params,
	NO_Returns,
)

FieldDoc( "Video.width", "The width of the video.",
	SeesDocs("Canvas|Image|Video.height|Video.width|Video.position|Video.duration|Video.metadata|Video.bitrate|Video.onframe|Video.canvas" ),
	NO_Examples,
	IS_Dynamic, IS_Public, IS_Readonly,
	"integer",
	NO_Default
)

FieldDoc( "Video.height", "The height of the video.",
	SeesDocs("Canvas|Image|Video.height|Video.width|Video.position|Video.duration|Video.metadata|Video.bitrate|Video.onframe|Video.canvas" ),
	NO_Examples,
	IS_Dynamic, IS_Public, IS_Readonly,
	"integer",
	NO_Default
)

for i in ["Video", "Audio" ]:
	FieldDoc( i + ".metadata", "An object with metadata that belong to this " + i ,
		SeesDocs("Canvas|Image|" + i + "|" + i + ".position|" + i + ".duration|" + i + ".metadata|" + i + ".bitrate" ),
		[ExampleDoc( """for (var k in source.metadata) {
		console.log("Metadata : " + k + "=" + source.metadata[k]);
	}""") ],
		IS_Dynamic, IS_Public, IS_Readonly,
		"Object",
		NO_Default
	)

EventDoc( "Video.onframe", "Function that will be executed on a onframe event.",
	NO_Sees,
	NO_Examples,
	[ ParamDoc( "video", "Video instance", "Video", NO_Default, IS_Obligated ) ]
)

ConstructorDoc( "Video", "Create a new Video instance.",
	[ SeeDoc( "Image" ), SeeDoc( "Audio" ) ],
	NO_Examples,
	[ParamDoc( "canvas", "The canvas parent. This must be a 2d Canvas", "Canvas", NO_Default, IS_Obligated ) ],
	ReturnDoc( "Video instance", "Video" )
)

FieldDoc( "Video.canvas", "The 2d canvas where this video was embedded on.",
	SeesDocs("Canvas|Image|Video.height|Video.width|Video.position|Video.duration|Video.metadata|Video.bitrate|Video.onframe|Video.canvas" ),
	NO_Examples,
	IS_Dynamic, IS_Public, IS_ReadWrite,
	"Canvas",
	NO_Default
)

for i in ["Video", "Audio" ]:
	FieldDoc( i +".position", "The current position in the " + i + " in seconds. Can also be used to set the position to a certain time.",
		SeesDocs("Canvas|Image|Video|Audio|Video.height|" + i + "width|" + i + "position|" + i + "duration|" + i + "metadata|" + i + "bitrate|Video.onframe|Video.canvas" ),
		[ ExampleDoc("""console.log(source.position); // display the current position in the source
source.position += 5; // seek 5 seconds forwards""")],
		IS_Dynamic, IS_Public, IS_ReadWrite,
		"integer",
		NO_Default
	)
	FieldDoc( i + ".duration", "The duration of the " + i + " in seconds.",
		SeesDocs("Canvas|Image|Video|Audio|Video.height|" + i + "width|" + i + "position|" + i + "duration|" + i + "metadata|" + i + "bitrate|Video.onframe|Video.canvas" ),
		NO_Examples,
		IS_Dynamic, IS_Public, IS_Readonly,
		"integer",
		NO_Default
	)
	FieldDoc( i +".bitrate", "The bitrate of the " + i + " in samples per buffer.",
		SeesDocs("Canvas|Image|Video|Audio|Video.height|" + i + "width|" + i + "position|" + i + "duration|" + i + "metadata|" + i + "bitrate|Video.onframe|Video.canvas" ),
		NO_Examples,
		IS_Dynamic, IS_Public, IS_Readonly,
		"integer",
		NO_Default
	)

FieldDoc( "AudioNode.seek", "Go to a certain position, starting from the start.",
	SeesDocs("Canvas|Image|Video|Audio|AudioNode.height|AudioNode.width|AudioNode.position|AudioNode.duration|AudioNode.metadata|AudioNode.bitrate" ),
	NO_Examples,
	IS_Dynamic, IS_Public, IS_Readonly,
	'integer'
	'0'
)


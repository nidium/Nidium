/* -----------------------------------------------------------------
 * Native Digital Signal Processing Draft                          *
 * ----------------------------------------------------------------- 
 * Version: 0.1                                                    *
 * ----------------------------------------------------------------- 
 */
/*
var t = new AudioThread(function(){
	var self = this,

		AudioIn = new Node("source", 0, 2),	// 0 IN, 2 OUT (stereo)
		Gain1 = new Node("gain", 1, 1), 	// 1 IN, 1 OUT (mono)
		Gain2 = new Node("gain", 1, 1), 	// 1 IN, 1 OUT (mono)
		BF = new Node(BFHandler, 2, 2),		// 2 IN, 2 OUT (stereo)
		Mixer = new Node("mixer", 2, 2),  	// 2 IN, 2 OUT (stereo)
		OUTPUT = new Node("target", 2, 0);  // 2 IN, 0 OUT (stereo)

	// init audiocontext with 512 bytes buffer, 2 channels, 16bits, 44100Hz
	self.init(512, 2, 16, 44100);

	// connect AudioIn to Gain Processors
	self.connect(AudioIn.channel(0), Gain1.channel(0));
	self.connect(AudioIn.channel(1), Gain2.channel(0));

	// connect Gain Processors to Custom JS BF filter
	self.connect(Gain1.channel(0), BF.channel(0));
	self.connect(Gain2.channel(0), BF.channel(1));

	// connect BF to Mixer
	self.connect(BF.channel(0), Mixer.channel(0));
	self.connect(BF.channel(1), Mixer.channel(1));

	// send Mixer to target audio device
	self.connect(Mixer.channel(0), OUTPUT.channel(0));
	self.connect(Mixer.channel(1), OUTPUT.channel(1));

	var loadAudioSource = function(url, cb){
		var r  = new Http(url).request(function(e){
			if (e.type == "audio"){
				cb(e.data);
			}
		}
	};

	var BFHandler = function(e){
		var inputL = e.input.channel(0),
			inputR = e.input.channel(1),

			outputL = e.output.channel(0),
			outputR = e.output.channel(1);

		// Basic Gain Filter
		for (var i=0; i<inputL.length; i++) {
			outputL[i] = 0.5 * inputL[i];
			outputR[i] = 0.5 * inputR[i];
		}

	};

	loadAudioSource("http://www.sof.com/blabla.mp3", function(audiobuffer){
	    AudioIn.buffer = audiobuffer;
	    AudioIn.loop = false;
	    AudioIn.noteOn(0);
		self.start();

		self.send({
			name : "start"
		});
	});

	self.onmessage = function(e){
		switch (e.name) {
			case "play" :
				AudioIn.noteOn(0);
				break;

			case "stop" :
				AudioIn.noteOff(0);
				break;

			case "gain" :
				Gain1.gain = e.value;
				Gain2.gain = e.value;
				break;
		}
	};

});
*/

/* receive messages from audio thread */
t.onmessage = function(e){
	if (e.name == 'start') {
		console.log("DSP started")
	}
};

var stopPlaying = function(){
	t.send({
		name : "stop"
	});
};

var setGain = function(g){
	t.send({
		name : "gain",
		value : g
	});
};


/* set Gain1 and Gain2 to 0.25 after 6 seconds */
setTimeout(function(){
	setGain(0.5);
}, 6000);


/* stop playing after 30 seconds */
setTimeout(function(){
	stopPlaying();
}, 30000);













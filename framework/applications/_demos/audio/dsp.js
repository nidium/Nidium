/* ------------------------+------------- */
/* Native Framework 2.0    | Falcon Build */
/* ------------------------+------------- */
/* (c) 2013 nidium.com - Vincent Fontaine */
/* -------------------------------------- */

/*
 * Native Featured Demo
 *
 * This tutorial will show you:
 *  - How to create a simple UI with a slider (UISliderController)
 *  - How to add event listeners to UI elements (addEventListerner method)
 * 	- How to make an HTTP request to fetch a distant MP3
 *  - How to use the basics of Native Audio DSP 
 */

var main = new Application({background:"#222233"});

var	slider = new UISliderController(main, {
	x : 350,
	y : 140,
	w : 250,
	h : 20,

	splitColor : 'rgba(255, 255, 255, 0.5)',
	boxColor : 'rgba(255, 255, 255, 0.02)',

	min : 0,
	max : 1,
	value : 0.5
});


/*
 * Native Audio DSP, the "Hello World Basics"
 */

var myAudioApp = {
	dsp : null,
	nodes : {},

	init : function(){
		// Init Native Digital Signal Processor
		// 1024 bytes buffer, 2 channels, 44100Hz
		this.dsp = new Audio(1024, 2, 44100);

		this.createNodes(); // create some nodes ...
		this.connectNodes(); // ... connect them together ...
	},

	createNodes : function(){
		// Create our audio nodes
		this.nodes = {
			source : this.dsp.createNode("source", 0, 2),
			gain   : this.dsp.createNode("gain", 2, 2),
			target : this.dsp.createNode("target", 2, 0)
		};


		this.nodes.gain.set("gain", 0.5);

	},

	connectNodes : function(){
		var	SOURCE = this.nodes.source,
			GAIN   = this.nodes.gain,
			TARGET = this.nodes.target;

		/*
		 
		 	Connect the nodes : SOURCE --> GAIN --> TARGET
		 
		   	+--------+    L    +------+    L    +--------+
		   	|        | ------> |      | ------> |        |
		   	| SOURCE |         | GAIN |         | TARGET |  
		   	|        | ------> |      | ------> |        | 
		   	+--------+    R    +------+    R    +--------+
		 
		*/

		// ... SOURCE ---> GAIN ..........................
		this.dsp.connect(SOURCE.output(0), GAIN.input(0));
		this.dsp.connect(SOURCE.output(1), GAIN.input(1));

		// ... GAIN ---> TARGET ..........................
		this.dsp.connect(GAIN.output(0), TARGET.input(0));
		this.dsp.connect(GAIN.output(1), TARGET.input(1));

	},

	load : function(url, callback){
		var self = this;
		var r = new Http(url).request(function(e){
			if (e.type == "audio"){
				callback.call(self, e.data);
			} else {
				throw "not audio data";
			}
		});
	},

	play : function(audioBuffer){
		this.nodes.source.open(audioBuffer);
		this.nodes.source.play();
	}	
};


//var tune = "http://195.122.253.112/public/mp3/Symphony%20X/Symphony%20X%20'Prelude'.mp3";
var tune = "http://labs.swelen.com/games/ztype4coders/media/music/endure.ogg";

myAudioApp.init();
myAudioApp.load(tune, function(audioBuffer){
	this.play(audioBuffer);
});

// now, attach a listener to our slider,
// set the gain node on change
slider.addEventListener("change", function(value){
	myAudioApp.nodes.gain.set("gain", value);
}, false);




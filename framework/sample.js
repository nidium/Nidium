
"use strict";

var main = new Application();

var mixer = null,
	tracks = [];

var tx = [
	{file : "media/endure.ogg",	label : "Depress"},
	{file : "media/endure.ogg",	label : "Depress"},
	{file : "media/endure.ogg",	label : "Depress"}
];


for (var i=0; i<tx.length; i++){
	tracks[i] = AudioMixer.load(tx[i].file, function(data, k){
		this.volume(0.5);
		this.play();
	}, i);


	tracks[i].processor.onbuffer = function(ev, scope){
		var channels = ev.data,
			gain = this.get("gain");

		for (var n=0; n<channels.length; n++){
			var buffer = channels[n];
			for (var i=0; i<buffer.length; i++){
				buffer[i] *= gain;
			}
		}
	};

	tracks[i].processor.onmessage = function(e){
		//console.log(e.data);
	};

}

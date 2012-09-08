/* -------------------------- */
/* Native (@) 2012 Stight.com */
/* -------------------------- */

var m = new MultiTrack();

m.track[0].apply({
	gain : {
		left : 0.5,
		right : 0.5
	},

	reverb : {
		damp : 0.5,
		size : 0.1,
		time : 0.03
	}
});

m.track[1].apply(task);
m.track[2].apply(task);
m.track[3].apply(task);
m.master.setVolume(0.5);

m.play(function(time){


});

var gain = new Thread(function(buffer, l, volume){
	for(var i=0 ; i<l ; i++){
		buffer[i] *= 0.5;
	}
});


var reverb = new Thread(function(buffer, l, volume){
	for(var i=0 ; i<l ; i++){
		/* reverb */
	}
});


tast.start();
task.pause();
task.onmessage(data);
task.onend




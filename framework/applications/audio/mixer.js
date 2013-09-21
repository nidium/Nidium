/* --------------------------------------------------------------------------- *
 * NATiVE MIXER DEMO                                       (c) 2013 nidium.com * 
 * --------------------------------------------------------------------------- * 
 * Version:     1.0                                                            *
 * Author:      Vincent Fontaine                                               *
 *                                                                             *
 * Permission is hereby granted, free of charge, to any person obtaining a     *
 * copy of this software and associated documentation files (the "Software"),  *
 * to deal in the Software without restriction, including without limitation   *
 * the rights to use, copy, modify, merge, publish, distribute, sublicense,    *
 * and/or sellcopies of the Software, and to permit persons to whom the        *
 * Software is furnished to do so, subject to the following conditions:        *
 *                                                                             *
 * The above copyright notice and this permission notice shall be included in  *
 * all copies or substantial portions of the Software.                         *
 *                                                                             *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR  *
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,    *
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE *
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER      *
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING     *
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER         *
 * DEALINGS IN THE SOFTWARE.                                                   *
 * --------------------------------------------------------------------------- * 
 */

"use strict";

var main = new Application();
main.backgroundImage = "private://assets/patterns/wood_1.png";

var mixer = null,
	master = null,
	tracks = [],
	slides = [];

var tx = [
	{file : "media/drums.mp3",		label : "Drums"},
	{file : "media/bass.mp3",		label : "Bass"},
	{file : "media/guitar01.mp3",	label : "Guitar"},
	{file : "media/vocals.mp3",		label : "Vocals"},
	{file : "media/sequences.mp3",	label : "Seq"},
	{file : "media/guitar02.mp3",	label : "Solo"},

	{file : "media/drums.mp3",		label : "Drums"},
	{file : "media/bass.mp3",		label : "Bass"},
	{file : "media/guitar01.mp3",	label : "Guitar"},
	{file : "media/vocals.mp3",		label : "Vocals"},
	{file : "media/sequences.mp3",	label : "Seq"},
	{file : "media/guitar02.mp3",	label : "Solo"}
];

/* -------------------------------------------------------------------------- */

var createMixer = function(){
	mixer = new UIView(main, {
		id : "audioMixerContainer",
		top : 300,
		left : 10,
		width : 1000,
		height : 350,
		background : "rgba(0, 0, 0, 0.25)",
		radius : 4
	}).centerLeft();

	mixer.contentView = new UIView(mixer, {
		width : mixer.width-64,
		height : mixer.height-8,
		overflow : false,
		scrollable : true
	});

	mixer.masterView = new UIView(mixer, {
		left : mixer.width-60,
		width : 60,
		height : mixer.height,
		background : 'rgba(100, 0, 0, 0.5)',
		overflow : false,
		radius : 4
	});
};

var createMasterSlider = function(){
	master = new UIView(mixer.masterView, {
		id : "masterSlide",
		width : 60,
		height : 336,
		left : 1,
		class : "slide"
	});

	master.panLabel = master.add("UILabel", {
		top : 0,
		label : "L                 R",
		fontSize : 9,
		class : "panLabel"
	}).centerLeft();

	master.hintLabel = master.add("UILabel", {
		width : 56,
		label : "MASTER",
		background : "rgba(0, 0, 0, 0.80)",
		shadowBlur : 30,
		class : "hintLabel"
	}).centerLeft();

	master.panSlider = master.add("UISliderController", {
		top : 6,
		width : 32,
		height : 8,
		color : "#440000",
		min : -1,
		max : 1,
		value : 0
	});
	master.panSlider.className = "panSlider";
	master.panSlider.centerLeft();

	master.levelSlider = master.add("UISliderController", {
		top : 50,
		width : 22,
		height : 250,
		color : "black",
		splitColor : 'rgba(140, 140, 140, 0.3)',
		boxColor : 'rgba(255, 255, 255, 0.07)',
		progressBarColor : 'rgba(20, 80, 250, 1)',
		radius : 2,
		vertical : true,
		min : 0.0,
		max : 2,
		value : 0.5
	}).centerLeft().addEventListener("change", function(ev){
	   AudioMixer.volume(this.value);
	}, false);

};

/* -------------------------------------------------------------------------- */

var createSlider = function(k, label){
	var r = Math.round(Math.random()*255),
		v = Math.round(Math.random()*255),
		b = Math.round(Math.random()*255);

	slides[k] = new UIView(mixer.contentView, {
		id : "audioSlide_"+k,
		width : 60,
		height : 336,
		left : 1+62*k,
		class : "slide"
	});

	slides[k].panLabel = slides[k].add("UILabel", {
		top : 0,
		label : "L                 R",
		fontSize : 9,
		class : "panLabel"
	}).centerLeft();

	slides[k].soloButton = new UIButton(slides[k], "solobutton");
	slides[k].soloButton.rel = k;
	slides[k].soloButton.addEventListener("mouseup", function(e){
		soloTrack(this);
	});

	slides[k].muteButton = new UIButton(slides[k], "mutebutton");
	slides[k].muteButton.rel = k;
	slides[k].muteButton.addEventListener("mouseup", function(e){
		muteTrack(this);
	});

	slides[k].hintLabel = slides[k].add("UILabel", {
		width : 56,
		label : k+". "+label,
		background : "rgba("+r+", "+v+", "+b+", 0.80)",
		shadowBlur : 30,
		class : "hintLabel"
	}).centerLeft();

	slides[k].hintLabel.angle = 3*(Math.random()*2-1);
	slides[k].hintLabel.top += 2*(Math.random()*2-1);

	slides[k].panSlider = slides[k].add("UISliderController", {
		top : 6,
		width : 32,
		height : 8,
		color : "#440000",
		min : -1,
		max : 1,
		value : 0
	});
	slides[k].panSlider.className = "panSlider";
	slides[k].panSlider.centerLeft();

	slides[k].levelSlider = slides[k].add("UISliderController", {
		top : 50,
		width : 22,
		height : 250,
		color : "black",
		splitColor : 'rgba(140, 140, 140, 0.3)',
		boxColor : 'rgba(255, 255, 255, 0.07)',
		progressBarColor : 'rgba(210, 255, 40, 1)',
		radius : 2,
		vertical : true,
		min : 0.0,
		max : 2,
		value : 0.5
	}).centerLeft().addEventListener("change", function(ev){
	   tracks[k].volume(this.value);
	}, false);
};

/* -------------------------------------------------------------------------- */

var muteTrack = function(obj){
	var k = obj.rel;
	if (obj.muted) {
		tracks[k].unmute();
		_unselect(obj);
		obj.muted = false;
	} else {
		tracks[k].mute();
		_select(obj);
		obj.muted = true;
	}
};

/* -------------------------------------------------------------------------- */

var soloTrack = function(obj){
	var k = obj.rel,
		thisMuteButton = null,

		muteButtons = document.getElementsByClassName("mutebutton"),
		soloButtons = document.getElementsByClassName("solobutton");

	if (obj.soloed) {
		obj.soloed = false;
		muteButtons.each(function(){
			_unselect(this);
		});
		soloButtons.each(function(){
			_unselect(this);
		});
	} else {
		muteButtons.each(function(){
			_grey(this);
			if (this.rel == k) thisMuteButton = this;
		});
		soloButtons.each(function(){
			this.soloed = false;
			_unselect(this);
		});

		obj.soloed = true;

		_select(obj);
		_unselect(thisMuteButton);
	}
	tracks[k].solo();
};

/* -------------------------------------------------------------------------- */

var _select = function(obj){
	obj.color = "#ffffff";
	obj.background = "red";
};

var _unselect = function(obj){
	obj.color = "#cccccc";
	obj.background = "rgba(33, 0, 0, 0.1)";
};

var _grey = function(obj){
	obj.color = "#888888";
	obj.background = "rgba(128, 0, 0, 0.5)";
};

/* -------------------------------------------------------------------------- */

var play = new UIButton(main, {
	label : "Play"
}).move(10, 8).click(function(e){
	AudioMixer.play();
});

var pause = new UIButton(main, {
	label : "Pause"
}).move(60, 8).click(function(e){
	AudioMixer.pause();
});

var stop = new UIButton(main, {
	label : "Stop"
}).move(120, 8).click(function(e){
	AudioMixer.stop();
});

createMixer();
createMasterSlider();

for (var i=0; i<tx.length; i++){
	createSlider(i, tx[i].label);

	tracks[i] = AudioMixer.load(tx[i].file, function(data, k){
		slides[k].levelSlider.progressBarColor = 'rgba(210, 255, 40, 1)';
		this.volume(0.5);
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
		console.log(e.message);
	};
}


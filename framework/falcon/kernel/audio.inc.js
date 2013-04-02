/* --------------------------------------------------------------------------- *
 * NATiVE AUDIO API                                        (c) 2013 Stight.com *
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

/* -------------------------------------------------------------------------- */

Native.getAudioDSP = function(){
	// Init NATiVE DSP
	// 512 bytes buffer, 2 channels, 44100Hz
	if (!Native.scope.Audio) return false;
	if (!Native.defaultAudioDSP) {
		Native.defaultAudioDSP = Audio.getContext(512, 2, 44100);
	}
	return Native.defaultAudioDSP;
};

var AudioMixer = {
	loaded : false,

	onload : function(tracks){},

	load : function(url, callback, rel){
		var self = this;

		if (!this.loaded){
			this.dsp = Native.getAudioDSP();
			this.master = this.dsp.createNode("gain", 2, 2);
			this.target = this.dsp.createNode("target", 2, 0);

			this.dsp.connect(this.master.output(0), this.target.input(0));
			this.dsp.connect(this.master.output(1), this.target.input(1));

			this.volume(1.0);

			this.tracks = [];
			this.nbtracks = 0;
			this.synchro = 0;
			this.loaded = true;
		}

		// Create new Source and Gain nodes */
		var source = this.dsp.createNode("source", 0, 2);
		var gain = this.dsp.createNode("gain", 2, 2);
		var processor = this.dsp.createNode("custom", 2, 2);

		/*
		 
		    Connect the nodes : SOURCE --> GAIN --> TARGET
		 
		   	+--------+    L    +------+    L    +--------+
		   	|        | ------> |      | ------> |        |
		   	| SOURCE |         | GAIN |         | TARGET |
		   	|        | ------> |      | ------> |        |
		   	+--------+    R    +------+    R    +--------+
		 
		*/


		// ... SOURCE ---> GAIN ..........................
		this.dsp.connect(source.output(0), gain.input(0));
		this.dsp.connect(source.output(1), gain.input(1));

		// ... GAIN ---> PROCESSOR ..........................
//		this.dsp.connect(gain.output(0), processor.input(0));
//		this.dsp.connect(gain.output(1), processor.input(1));

		// ... GAIN ---> TARGET ...............................
		this.dsp.connect(gain.output(0), this.master.input(0));
		this.dsp.connect(gain.output(1), this.master.input(1));

		var r = {
			dsp : this.dsp,
			source : source,
			gain : gain,
			processor : processor,
			master : this.master,
			target : this.target,

			opened : false,
			playing : false,
			paused : false,
			muted : false,
			soloed : false,

			level : 0.001,
			data : null,

			rel : rel,

			open : function(data){
				if (this.opened) return this;
				this.opened = true;
				this.source.open(data);
				this.data = data;
				return this;
			},

			play : function(){
				if (this.playing) return this;
				if (this.paused) {
					this.unmute();
					this.paused = false;
				}
				this.playing = true;
				this.gain.set("gain", this.level);
				this.source.play();
				return this;
			},

			stop : function(){
				if (!this.playing) return this;

				this.opened = false;
				this.playing = false;
				this.paused = false;

				this.gain.set("gain", 0.0);
				this.source.stop();
				return this;
			},

			pause : function(){
				if (this.paused) return this;
				this.paused = true;
				this.playing = false;
				this.mute();
				this.source.pause();
				return this;
			},

			mute : function(){
				if (this.muted) return this;
				this.muted = true;
				this.soloed = false;
				this.gain.set("gain", 0.0);
				return this;
			},

			unmute : function(){
				if (!this.muted) return this;
				this.muted = false;
				this.gain.set("gain", this.level);
				return this;
			},

			solo : function(){
				if (this.soloed) {
					this.soloed = false;
					self.unmute();
				} else {
					self.mute();
					this.unmute();
					this.soloed = true;
				}
				return this;
			},

			volume : function(value){
				this.gain.set("gain", value);
				this.level = value;
				return this;
			}
		};

		this.tracks.push(r);
		this.nbtracks++;

		File.read(url, function(data, size){
			source.open(data);
			r.volume(0.5);
			self.synchro++;
			if (typeof callback === "function"){
				callback.call(r, data, r.rel);
			}
			if (self.synchro >= self.nbtracks && typeof self.onload === "function"){
				self.onload.call(self, self.tracks);
			}
		});

		return r;
	},

	play : function(){
		if (!this.tracks) return false;
		for (var i=0; i<this.tracks.length; i++){
			this.tracks[i].play();
		}
	},

	pause : function(){
		if (!this.tracks) return false;
		for (var i=0; i<this.tracks.length; i++){
			this.tracks[i].pause();
		}
	},

	stop : function(){
		if (!this.tracks) return false;
		for (var i=0; i<this.tracks.length; i++){
			this.tracks[i].stop();
		}
	},

	mute : function(){
		if (!this.tracks) return false;
		for (var i=0; i<this.tracks.length; i++){
			this.tracks[i].mute();
		}
	},

	unmute : function(){
		if (!this.tracks) return false;
		for (var i=0; i<this.tracks.length; i++){
			this.tracks[i].unmute();
		}
	},

	volume : function(value){
		this.master.set("gain", value);
		this.level = value;
	}
};

/* -------------------------------------------------------------------------- */


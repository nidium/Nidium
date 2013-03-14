/* --------------------------------------------------------------------------- *
 * NATiVE VIDEO API                                        (c) 2013 Stight.com *
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

var VideoLayer = function(layer, url, callback){
	var self = this,
		cb = OptionalCallback(callback, null);

	this.layer = layer;
	this.dsp = Native.getAudioDSP();
	if (!this.dsp) throw("Unsupported Native Audio DSP");
	this.audioGain = this.dsp.createNode("gain", 2, 2);
	this.audioTarget = this.dsp.createNode("target", 2, 0);

	this.layer.ctx.imageSmoothingEnabled = true;

	this.video = new Video(this.dsp, this.layer);
	this.video.onerror = this.onerror;

	File.read(url, function(data, size) {
		self.video.open(data);
		self.ready = true;

		var source = self.video.getAudioNode(),
			gain = self.audioGain,
			target = self.audioTarget;

		if (source != null) {
			// ... SOURCE ---> GAIN ..........................
			self.dsp.connect(source.output(0), gain.input(0));
			self.dsp.connect(source.output(1), gain.input(1));

			// ... GAIN ---> TARGET ..........................
			self.dsp.connect(gain.output(0), target.input(0));
			self.dsp.connect(gain.output(1), target.input(1));
		}

		if (cb) {
			cb.call(self, {
				size : size,
				data : data,
				width : self.video.width,
				height : self.video.height,
				metadata : self.video.metadata,
				duration : self.video.duration
			});
		}
	});
};

VideoLayer.prototype = {
	play : function(){
		var self = this;
		if (!this.ready || this.playing) return false;

		this.video.play();
		this.playing = true;
		this.paused = false;
		this.stoped = false;

		this.onplay();

		this.timer = setInterval(function(){
			self.onplaying({
				duration : self.duration,
				position : self.position,
				percent : Math.round(self.position*10000 / self.duration)/100
			});
		}, 64);
	},

	pause : function(){
		if (this.paused) return false;
		this.paused = true;
		this.stoped = false;
		this.playing = false;

		this.onpause();
		this.video.pause();
		clearInterval(this.timer);
	},

	stop : function(){
		if (this.stoped) return false;
		this.stoped = true;
		this.paused = false;
		this.playing = false;

		this.onstop();
		this.video.stop();
	},

	onplay : function(){},
	onpause : function(){},
	onstop : function(){},
	onplaying : function(e){},
	onbufferred : function(){},
	onerror : function(){},

	get volume(){
		return this.audioGain.get("gain");
	},

	set volume(gain){
		this.audioGain.set("gain", gain);
	},

	get metadata(){
		return this.video.metadata;
	},

	get duration(){
		return this.video.duration;
	},

	get position(){
		return this.video.position;
	},

	set position(position){
		this.video.position = position;
	}
};
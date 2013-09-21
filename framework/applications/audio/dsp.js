/* --------------------------------------------------------------------------- *
 * NATiVE DSP DEMO                                         (c) 2013 nidium.com * 
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

var main = new Application();
var	k2PI = 2.0 * Math.PI;

var app = {
	audioBuffer : 4096,

	dftSize : 128,
	dftBuffer : new Float64Array(128),

	init : function(){
		this.createNodes();
		this.connectNodes();
		this.setProcessor();
		this.attachListeners();

		/*
		FIX ME
		fileselector.click(function(){
			window.openFileDialog(["mp3", "wav", "mov", "aiff"], function(res){
				app.load(res[0]);
			});
		});
		*/

		this.load("http://195.122.253.112/public/mp3/Symphony%20X/Symphony%20X%20'Candlelight%20Fantasia'.mp3");

	},

	createNodes : function(){
		this.dsp = Audio.getContext(this.audioBuffer, 2, 44100);
		this.source = this.dsp.createNode("source", 0, 2);
		this.gain = this.dsp.createNode("gain", 2, 2);
		this.processor = this.dsp.createNode("custom", 2, 2);
		this.delay = this.dsp.createNode("delay", 2, 2);
		this.target = this.dsp.createNode("target", 2, 0);
		console.log("nodes created");
	},

	load : function(url){
		var self = this;
		/*
		File.read(url, function(data){
			self.source.open(data);
			self.source.play();
		});
		*/
		setTimeout(function(){
			self.source.open(url);
			self.source.play();
		}, 200);
	},

	setProcessor : function(){
		var self = this;

		this.processor.oninit = function(scope){
			scope.ResonantFilter = function(type, sampleRate){
				this.type = type;
				this.sampleRate = sampleRate;

				this.f = new Float32Array(4);
				this.f[0] = 0.0; // lp
				this.f[1] = 0.0; // hp
				this.f[2] = 0.0; // bp
				this.f[3] = 0.0; // br
			};

			scope.ResonantFilter.prototype.update = function(cutoff, resonance){
				this.cutoff = cutoff;
				this.resonance = resonance;
				
				this.freq = 2 * Math.sin(
					Math.PI * Math.min(0.25, cutoff/(this.sampleRate*2))
				);

				this.damp = Math.min(
					2 * (1 - Math.pow(resonance, 0.25)),
					Math.min(2, 2/this.freq - this.freq * 0.5)
				);

			};

			scope.ResonantFilter.prototype.process = function(input){
				var output = 0,
					f = this.f;

				// first pass
				f[3] = input - this.damp * f[2];
				f[0] = f[0] + this.cutoff * f[2];
				f[1] = f[3] - f[0];
				f[2] = this.cutoff * f[1] + f[2];
				output = 0.5 * f[this.type];

				// second pass
				f[3] = input - this.damp * f[2];
				f[0] = f[0] + this.cutoff * f[2];
				f[1] = f[3] - f[0];
				f[2] = this.cutoff * f[1] + f[2];
				output += 0.5 * f[this.type];

				return output;
			};

			scope.MoogFilter = function(){
				this.in1 = 0.0;
				this.in2 = 0.0;
				this.in3 = 0.0;
				this.in4 = 0.0;

				this.out1 = 0.0;
				this.out2 = 0.0;
				this.out3 = 0.0;
				this.out4 = 0.0;
			};

			scope.MoogFilter.prototype.update = function(cutoff, resonance){
				this.cutoff = cutoff;
				this.resonance = resonance;
			};

			scope.MoogFilter.prototype.process = function(input){
				var f = this.cutoff * 1.16,
					ff = f * f;

				var fb = this.resonance * (1.0 - 0.15 * ff);

				input -= this.out4 * fb;
				input *= 0.35013 * (ff*ff);

				this.out1 = input + 0.3 * this.in1 + (1 - f) * this.out1; // Pole 1
				this.in1  = input;

				this.out2 = this.out1 + 0.3 * this.in2 + (1 - f) * this.out2;  // Pole 2
				this.in2  = this.out1;

				this.out3 = this.out2 + 0.3 * this.in3 + (1 - f) * this.out3;  // Pole 3
				this.in3  = this.out2;

				this.out4 = this.out3 + 0.3 * this.in4 + (1 - f) * this.out4;  // Pole 4
				this.in4  = this.out3;

				return this.out4;
			};

			scope.StereoWidth = function(){
				this.width = 0;
			};

			scope.StereoWidth.prototype.update = function(width){
				this.width = width;
			};

			scope.StereoWidth.prototype.process = function(inL, inR){
				var scale = this.width * 0.5,
					m = (inL + inR) * 0.5,
					s = (inR - inL) * scale;

				return [m-s, m+s];
			};

			scope.step = 0;
			scope.moogFilterL = new scope.MoogFilter();
			scope.moogFilterR = new scope.MoogFilter();
			scope.enhancer = new scope.StereoWidth();
			scope.resonantL = new scope.ResonantFilter(0, 44100);
			scope.resonantR = new scope.ResonantFilter(0, 44100);
		};

		/* Threaded Audio Processor */
		this.processor.onbuffer = function(ev, scope){
			var processor = this,
				bufferL = ev.data[0],
				bufferR = ev.data[1],
				size = bufferL.length,

				gain = this.get("gain"),
				cutoff = this.get("cutoff"),
				resonance = this.get("resonance"),
				width = this.get("width");

			var echo = function(txt){
				processor.send(txt);
			};

			scope.resonantL.update(cutoff, resonance);
			scope.resonantR.update(cutoff, resonance);
			scope.enhancer.update(width);

			for (var i=0; i<size; i++) {
				var L = gain * scope.resonantL.process(bufferL[i]);
				var R = gain * scope.resonantR.process(bufferR[i]);

				var eh = scope.enhancer.process(L, R);

				bufferL[i] = eh[0];
				bufferR[i] = eh[1];
			}

			if ((scope.step++) % 3 == 0) {
				this.send({
					bufferL : bufferL,
					bufferR : bufferR
				});
			}
		};

		this.processor.onmessage = function(e){
			if (typeof e.data == "string" || typeof e.data == "number") {
				console.log(e.data);
			} else {
				self.iqDFT(e.data.bufferL, e.data.bufferR);
			}
		};

		this.processor.set("gain", 0.5);
		this.processor.set("cutoff", 0.04);
		this.processor.set("resonance", 0.0);
		this.processor.set("width", 1.00);

		console.log("processor initied");

	},


	/*
	 * UltraFast Magic DFT by Iñigo Quílez
	 * JS implementation by Vincent Fontaine
	 * http://www.iquilezles.org/www/articles/sincos/sincos.htm
	 */
	iqDFT : function(bufferL, bufferR){
		var bufferSize = bufferL.length, //256
			len = bufferSize, // 256
			angularNormalisation = k2PI/(len); // len

		for(var i=0; i<this.dftSize; i++){
			var wi = i * angularNormalisation,
				sii = Math.sin(wi),	coi = Math.cos(wi),
				co = 1.0, si = 0.0,	acco = 0.0, acsi = 0.0;

			for(var j=0; j<bufferSize; j++){
				var f = bufferL[j] + bufferR[j],
					oco = co;

				acco += co*f; co = co*coi -  si*sii;
				acsi += si*f; si = si*coi + oco*sii;
			}
			this.dftBuffer[i] = Math.sqrt(acco*acco + acsi*acsi) * 1/128;
		}
		Spectral.ondata();
	},

	connectNodes : function(){
		this.dsp.connect(this.source.output(0), this.gain.input(0));
		this.dsp.connect(this.source.output(1), this.gain.input(1));

		this.dsp.connect(this.gain.output(0), this.processor.input(0));
		this.dsp.connect(this.gain.output(1), this.processor.input(1));

		this.dsp.connect(this.processor.output(0), this.delay.input(0));
		this.dsp.connect(this.processor.output(1), this.delay.input(1));

		this.dsp.connect(this.delay.output(0), this.target.input(0));
		this.dsp.connect(this.delay.output(1), this.target.input(1));

		this.delay.set("dry", 0.9); // float 0 ... 1
		this.delay.set("wet", 0.0); // float 0 ... 1
		this.delay.set("delay", 500); // delay in ms
		console.log("nodes created");
	},

	attachListeners : function(){
		console.log("attaching listeners");

		this.source.onplay = function(){
			console.log("Playing");
		};

		this.source.onstop = function(){
			console.log("Stopped");
		};

		this.source.onerror = function(err){
			console.log("Error : " + err);
		};

		this.source.onbuffering = function(value){
			console.log("Buffering : " + value);
		};
	}
};


var Spectral = {
 	loga : false,
 	step : 0,

 	UIBars : [],
 	bars : [],

 	nbars : app.dftSize,
	barSize : 200,

	cw : 768,
	ch : 250,

	barW : 0,
	barY : 0,

	gdBack : null,
	gdSpectrum : null,

	init : function(){
		var self = this;

		this.spectrum = main.add("UIView", {
			left : 0,
			top : 0,
			width : this.cw,
			height : this.ch,
			background : "black",
			shadowBlur : 12,
			opacity : 0.9,
			overflow : false
		}).center();

		this.spectrum.addEventListener("drag", function(e){
			this.left += e.xrel;
			this.top += e.yrel;
		});

		this.spectrum.volumeSlider = this.spectrum.add("UISliderController", {
			left : 16,
			top : 10,
			width : 224,
			height : 16,

			fontSize : 10,
			lineHeight : 18,
			color : "black",

			splitColor : 'rgba(0, 0, 0, 0.5)',
			boxColor : 'rgba(255, 255, 255, 0.02)',

			radius : 2,
			min : 0.001,
			max : 3,
			value : 0.5
		});

		this.spectrum.cutoffSlider = this.spectrum.add("UISliderController", {
			left : 16,
			top : 30,
			width : 224,
			height : 16,

			fontSize : 10,
			lineHeight : 18,
			color : "black",

			splitColor : 'rgba(0, 0, 0, 0.5)',
			boxColor : 'rgba(255, 255, 255, 0.02)',

			radius : 2,
			min : 0.02,
			max : 0.4,
			value : 0.08
		});

		this.spectrum.rezoSlider = this.spectrum.add("UISliderController", {
			left : 16,
			top : 50,
			width : 224,
			height : 16,

			fontSize : 10,
			lineHeight : 18,
			color : "black",

			splitColor : 'rgba(0, 0, 0, 0.5)',
			boxColor : 'rgba(255, 255, 255, 0.02)',

			radius : 2,
			min : 0,
			max : 0.8,
			value : 0
		});


		this.spectrum.delayTime = this.spectrum.add("UISliderController", {
			left : 400,
			top : 10,
			width : 120,
			height : 12,

			fontSize : 10,
			lineHeight : 18,
			color : "red",
			splitColor : 'rgba(0, 0, 0, 0.5)',
			boxColor : 'rgba(255, 255, 255, 0.02)',

			radius : 2,
			min : 0.0,
			max : 1500,
			value : 500
		});
		this.spectrum.delayTime.addEventListener("change", function(e){
			app.delay.set("delay", this.value);
		}, false);

		this.spectrum.delayWet = this.spectrum.add("UISliderController", {
			left : 400,
			top : 30,
			width : 120,
			height : 12,

			fontSize : 10,
			lineHeight : 18,
			color : "red",
			splitColor : 'rgba(0, 0, 0, 0.5)',
			boxColor : 'rgba(255, 255, 255, 0.02)',

			radius : 2,
			min : 0.0,
			max : 1.0,
			value : 0.0
		});
		this.spectrum.delayWet.addEventListener("change", function(e){
			app.delay.set("wet", this.value);
		}, false);


		this.spectrum.stereoSlider = this.spectrum.add("UISliderController", {
			left : 400,
			top : 50,
			width : 120,
			height : 12,

			fontSize : 10,
			lineHeight : 18,
			color : "blue",
			splitColor : 'rgba(0, 0, 0, 0.5)',
			boxColor : 'rgba(255, 255, 255, 0.02)',

			radius : 2,
			min : 0.0,
			max : 3.0,
			value : 1.0
		});
		this.spectrum.stereoSlider.addEventListener("change", function(e){
			app.processor.set("width", this.value);
		}, false);


		this.spectrum.volumeSlider.addEventListener("change", function(e){
			app.processor.set("gain", this.value);
		}, false);

		this.spectrum.cutoffSlider.addEventListener("change", function(e){
			app.processor.set("cutoff", this.value);
		}, false);

		this.spectrum.rezoSlider.addEventListener("change", function(e){
			app.processor.set("resonance", this.value);
		}, false);

		this.barW = this.cw / this.nbars;
		this.barY = this.ch - this.barSize;

		this.gdBack = this.spectrum.layer.context.createLinearGradient(0, 0, 0, this.ch),
		this.gdSpectrum = this.spectrum.layer.context.createLinearGradient(0, this.barY, 0, this.ch);

		this.gdBack.addColorStop(0.00,'#444444');
		this.gdBack.addColorStop(1.00,'#000000');
		this.spectrum.background = this.gdBack;

		this.gdSpectrum.addColorStop(0.00,'#ff0000');
		this.gdSpectrum.addColorStop(0.25,'#ffff00');
		this.gdSpectrum.addColorStop(0.50,'#00ff00');
		this.gdSpectrum.addColorStop(0.75,'#00dddd');
		this.gdSpectrum.addColorStop(1.00,'#000055');

		this.clearBuffer();

		for (var i=0 ; i<this.nbars ; i++){
			this.UIBars[i] = this.spectrum.add("UIElement", {
				left : 1 + i * this.barW,
				top : this.ch-1,
				width : this.barW - 2,
				height : this.barSize,
				background : this.gdSpectrum,
				shadowBlur : 6,
				shadowOffsetY : 4,
				shadowColor : 'rgba(255, 100, 100, 0.8)'
			});
		}

		this.makeBars();

		var	fileselector = new UIButton(main, {
			left : window.width-58,
			top : 8,
			label : "Select"
		});

		app.init();
		this.draw();

	},

	clearBuffer : function(){
		for (var i=0; i<this.nbars; i++){
			app.dftBuffer[i] = 0;
		}
	},

	ondata : function(){
		var value = 0,
			pixelSize = 0;

		for (var i=0; i<this.nbars; i++){
			value = app.dftBuffer[i],
			pixelSize = value*this.barSize>>0;

			this.UIBars[i].top = this.ch-pixelSize;
		}
	},

	makeBars : function(){
		if (this.loga === true){
			for (var i=1 ; i<this.nbars ; i++){
				this.bars[i] = {
					x : 1 + Math.log(i)/Math.log(this.nbars)*this.cw,
					y : this.barY,
					w : this.barW - 1,
					h : this.barSize
				};
			}
		} else {
			for (var i=0 ; i<this.nbars ; i++){
				this.bars[i] = {
					x : 1 + i * this.barW,
					y : this.barY,
					w : this.barW - 1,
					h : this.barSize
				};
			}
		}
	},

	peak : function(s, l){
		var basspeak = 0;
		for (var b=s; b<(s+l); b++){
			basspeak += app.dftBuffer[b]/l;
		}
		if (basspeak>0.40){
			this.spectrum.background = '#666666';
			return '#ffffff';
		} else {
			this.spectrum.background = '';
			return this.gdBack;
		}
	},

	draw : function(){
		window.requestAnimationFrame(this.draw);

		var	width = this.cw,
			height = this.ch,
			params = this.spectrum.getDrawingBounds(),
			context = this.spectrum.layer.context;


		//this.spectrum.layer.clear();
		context.globalAlpha = 0.6;

		context.fillStyle = this.gdBack;
		//context.fillStyle = this.peak(14, 10);

		context.fillRect(0, 0, width, height);
		context.fillStyle = this.gdSpectrum;

		context.strokeStyle = "rgba(255, 255, 255, 0.6)";
		context.lineWidth = 1;

		context.beginPath();
		context.moveTo(0, height);
		for (var i=(this.loga?1:0), pixelSize=0 ; i<this.nbars ; pixelSize = app.dftBuffer[i] * this.barSize, i++){
			if (this.loga){
				this.bars[i].h = Math.log(i)/Math.log(0.5*pixelSize)*this.barSize
			} else {
				this.bars[i].h = pixelSize;
			}

			this.bars[i].y = height-pixelSize;

			context.fillRect(this.bars[i].x, this.bars[i].y, this.bars[i].w, this.bars[i].h);
			//context.lineTo(this.bars[i].x, this.bars[i].y);
		}
		context.lineTo(width, height);
		/*
		context.lineTo(0, height);
		context.stroke();
		*/
		context.fill();

		context.strokeStyle = "#000000";
		context.lineWidth = 1;
		for (var y=0; y<height; y+=3){
			context.beginPath();
			context.moveTo(0, y);
			context.lineTo(width, y);
			context.stroke();
		}

		if (this.loga){
			var t0 = 0.1655*width, // 55Hz A0
				t1 = 0.2745*width, // 110Hz A1
				t2 = 0.3835*width, // 220Hz A2
				t3 = 0.4925*width, // 440Hz A3
				t4 = 0.6015*width, // 880Hz A4
				t5 = 0.7105*width, // 1760Hz A5
				t6 = 0.8195*width, // 3520Hz A6
				t7 = 0.9285*width; // 7040Hz A7

			context.strokeStyle = "rgba(255, 255, 255, 0.4)";
			context.beginPath();
			context.moveTo(t0, height-4);context.lineTo(t0, height);
			context.moveTo(t1, height-4);context.lineTo(t1, height);
			context.moveTo(t2, height-4);context.lineTo(t2, height);
			context.moveTo(t3, height-4);context.lineTo(t3, height);
			context.moveTo(t4, height-4);context.lineTo(t4, height);
			context.moveTo(t5, height-4);context.lineTo(t5, height);
			context.moveTo(t6, height-4);context.lineTo(t6, height);
			context.moveTo(t7, height-4);context.lineTo(t7, height);
			context.stroke();
		}
	}


};

Spectral.init();


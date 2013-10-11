/* --------------------------------------------------------------------------- *
 * NIDIUM DSP DEMO                                         (c) 2013 nidium.com * 
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

document.nss.add({
	"UISliderController" : {
		left : 130,
		width : 160,
		height : 12,
		color : "black"
	},

	".label" : {
		color : "rgba(255, 255, 255, 0.25)",
		autowidth : false,
		width : 120,
		height : 19,
		textAlign : "right",
		paddingRight : 10,
		background : "rgba(0, 0, 0, 0.4)"
	}	
});

var main = new Application();
main.backgroundImage = "private://assets/patterns/wood_1.png";

var app = {
	audioBufferSize : 2048,
	ipulse : 0,

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

		this.load("../../media/dream.mp3");

	},

	createNodes : function(){
		this.dsp = Audio.getContext(this.audioBufferSize, 2, 44100);
		this.source = this.dsp.createNode("source", 0, 2);
		this.gain = this.dsp.createNode("gain", 2, 2);
		this.processor = this.dsp.createNode("custom", 2, 2);
		this.delay = this.dsp.createNode("delay", 2, 2);
		this.target = this.dsp.createNode("target", 2, 0);
		console.log("nodes created");
		Audio.lib && (this.dsp.run(Audio.lib));
	},

	load : function(url){
		var self = this;
		
		File.read(url, function(data){
			self.source.open(data);
			self.source.play();
		});
	},

	setProcessor : function(){
		var self = this;

		this.processor.oninit = function(scope){
			scope.step = 0;

			scope.moogFilterL = new scope.MoogFilter();
			scope.moogFilterR = new scope.MoogFilter();

			scope.enhancer = new scope.StereoWidth();

			scope.resonantL = new scope.ResonantFilter(0, 44100);
			scope.resonantR = new scope.ResonantFilter(0, 44100);

			scope.compL = new scope.Compressor();
			scope.compR = new scope.Compressor();

			scope.synthL = new scope.LFO(44100, "pulse", 440);
			scope.synthR = new scope.LFO(44100, "pulse", 440);

			scope.distL = new scope.Distorsion();
			scope.distR = new scope.Distorsion();

			scope.envelope = new scope.ADSR(44100);
		};

		/* Threaded Audio Processor */
		this.processor.onbuffer = function(ev, scope){
			var processor = this,
				bufferL = ev.data[0],
				bufferR = ev.data[1],
				samples = bufferL.length,

				gain = this.get("gain"),
				cutoff = this.get("cutoff"),
				resonance = this.get("resonance"),
				width = this.get("width"),

				comp_scale = this.get("comp_scale"),
				comp_gain = this.get("comp_gain"),

				preamp = this.get("preamp"),

				noteon = this.get("noteon"),
				A = this.get("adsr_A"),
				D = this.get("adsr_D"),
				S = this.get("adsr_S"),
				R = this.get("adsr_R");


			scope.resonantL.update(cutoff, resonance);
			scope.resonantR.update(cutoff, resonance);

			scope.compL.update(comp_scale, comp_gain);
			scope.compR.update(comp_scale, comp_gain);

			scope.distL.update(preamp);
			scope.distR.update(preamp);

			scope.enhancer.update(width);
			scope.envelope.update(noteon, A, D, S, R);

			for (var i=0; i<samples; i++) {
				var L = gain * scope.distL.process(bufferL[i]);
				var R = gain * scope.distR.process(bufferR[i]);

				L = scope.compL.process(L);
				R = scope.compR.process(R);

				L = scope.resonantL.process(L);
				R = scope.resonantR.process(R);

				var eh = scope.enhancer.process(L, R),
					volume = scope.envelope.process();

				bufferL[i] = eh[0] * volume;
				bufferR[i] = eh[1] * volume;
			}

			/* --- LOW FREQUENCY OSCILLATOR --- */
			/*
			for (var i=0; i<samples; i++) {
				scope.synthL.generate();
				scope.synthR.generate();
				var L = gain * scope.synthL.getSample();
				var R = gain * scope.synthL.getSample();

				bufferL[i] = L;
				bufferR[i] = R;
			}
			*/


			if ((scope.step++) % 2 == 0) {
				this.send({
					bufferL : bufferL,
					bufferR : bufferR
				});
			}
		};

		this.processor.onmessage = function(e){
			Audio.iqDFT(e.data.bufferL, e.data.bufferR, self.dftBuffer);
			Spectral.ondata();
		};

		this.processor.set("gain", 0.7);
		this.processor.set("cutoff", 0.2);
		this.processor.set("resonance", 0.0);
		this.processor.set("width", 1.00);

		this.processor.set("comp_scale", 2.50);
		this.processor.set("comp_gain", 1.00);

		this.processor.set("preamp", 1.50);

		this.processor.set("noteon", false);
		this.processor.set("adsr_A", 10);
		this.processor.set("adsr_D", 0);
		this.processor.set("adsr_S", 1);
		this.processor.set("adsr_R", 10);

		console.log("processor initied");

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
	
	controls : [],
	controlCount : 0,

 	bars : [],

 	nbars : app.dftSize,
	barSize : 150,

	cw : 768,
	ch : 250,

	barW : 0,
	barY : 0,

	gdBack : null,
	gdSpectrum : null,

	createSpectrumView : function(){
		this.toolkit = main.add("UIElement", {
			top : 100,
			width : 300,
			height : 20,
			background : "#282828",
			shadowBlur : 24,
			shadowOffsetY : 10,
			shadowColor : "black",
			opacity : 0.9,
			overflow : false
		}).centerLeft();

		this.toolkit.addEventListener("drag", function(e){
			this.left += e.xrel;
			this.top += e.yrel;
		});

		this.spectrum = main.add("UIElement", {
			top : 400,
			width : this.cw,
			height : this.ch,
			background : "black",
			shadowBlur : 12,
			opacity : 0.9,
			overflow : false
		}).centerLeft();

		this.spectrum.addEventListener("drag", function(e){
			this.left += e.xrel;
			this.top += e.yrel;
		});

		main.shader("pulse.s", function(program, uniforms){
			var t = 0;
			setInterval(function(){
				uniforms.itime = t++;
				uniforms.ipulse = app.ipulse;
			}, 16);
		});
	},

	addControl : function(label, min, max, val, changeCallback){
		var top = this.controlCount * 20;

		var control = this.toolkit.add("UILabel", {
			class : "label",
			top : top,
			label : label
		});
		control.slider = this.toolkit.add("UISliderController", {
			top : top+4,
			min : min,
			max : max,
			value : val
		}).addEventListener("change", function(e){
			if (typeof(changeCallback) == "function") {
				changeCallback.call(this, this.value);
			}
		}, false);

		this.controls.push(control);
		this.controlCount++;
		this.toolkit.height = this.controlCount*20 - 1;
	},

	createControllers : function(){
		this.addControl("Preamp", 0.001, 25.0, 0.001, function(value){
			app.processor.set("preamp", value);
		});

		this.addControl("Volume", 0.001, 1.1, 0.7, function(value){
			app.processor.set("gain", value);
		});

		this.addControl("CutOff Frequency", 0.02, 0.4, 0.2, function(value){
			app.processor.set("cutoff", value);
		});

		this.addControl("Resonance", 0.0, 0.7, 0.0, function(value){
			app.processor.set("resonance", value);
		});

		this.addControl("Delay Time", 0, 1500, 500, function(value){
			app.delay.set("delay", value);
		});

		this.addControl("Delay Wet", 0.0, 1.0, 0.0, function(value){
			app.delay.set("wet", value);
		});

		this.addControl("StereoWidth", 0.0, 3.0, 1.0, function(value){
			app.processor.set("width", value);
		});

		this.addControl("Dynamic Threshold", 0.75, 2.5, 2.5, function(value){
			app.processor.set("comp_scale", value);
		});

		this.addControl("Dynamic Gain", 0.1, 1.5, 1.0, function(value){
			app.processor.set("comp_gain", value);
		});

		this.addControl("A", 0, 2000, 10, function(value){
			app.processor.set("adsr_A", value);
		});
		this.addControl("D", 0, 2000, 0, function(value){
			app.processor.set("adsr_D", value);
		});
		this.addControl("S", 0, 1, 1, function(value){
			app.processor.set("adsr_S", value);
		});
		this.addControl("R", 0, 3000, 10, function(value){
			app.processor.set("adsr_R", value);
		});

	},

	createGradients : function(){
		/* Spectrum Background Gradient */
		this.gdBack = this.spectrum.layer.context.createLinearGradient(0, 0, 0, this.ch),
		this.gdBack.addColorStop(0.00,'#444444');
		this.gdBack.addColorStop(1.00,'#000000');

		/* Multicolor Bar Gradient */
		this.gdSpectrum = this.spectrum.layer.context.createLinearGradient(0, this.barY, 0, this.ch);
		this.gdSpectrum.addColorStop(0.00,'#ff0000');
		this.gdSpectrum.addColorStop(0.25,'#ffff00');
		this.gdSpectrum.addColorStop(0.50,'#00ff00');
		this.gdSpectrum.addColorStop(0.75,'#00dddd');
		this.gdSpectrum.addColorStop(1.00,'#000055');
	},

	createBars : function(){
		this.barW = this.cw / this.nbars;
		this.barY = this.ch - this.barSize;

		if (this.loga === true){
			for (var i=1, log=Math.log ; i<this.nbars ; i++){
				this.bars[i] = {
					x : 1 + log(i)/log(this.nbars)*this.cw,
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

	init : function(){
		this.createSpectrumView();
		this.createControllers();
		this.createGradients();
		this.createBars();

		this.clearDFTBuffer();

		var	fileselector = new UIButton(main, {
			left : window.width-58,
			top : 8,
			label : "Select"
		});

		var	noteon = new UIButton(main, {
			left : window.width-64,
			top : 34,
			label : "NoteOn"
		});

		noteon.addEventListener("mousedown", function(){
			app.processor.set("noteon", true);
		});
		noteon.addEventListener("mouseup", function(){
			app.processor.set("noteon", false);
		});

		app.init();
		this.draw();
	},

	clearDFTBuffer : function(){
		for (var i=0; i<this.nbars; i++){
			app.dftBuffer[i] = 0;
		}
	},

	ondata : function(){
	},

	peak : function(s, l){
		var basspeak = 0;
		for (var b=s; b<(s+l); b++){
			basspeak += app.dftBuffer[b]/l;
		}
		app.ipulse = basspeak*700;
	},

	draw : function(){
		window.requestAnimationFrame(this.draw.bind(this));

		var	width = this.cw,
			height = this.ch,
			params = this.spectrum.getDrawingBounds(),
			context = this.spectrum.layer.context;

		app.ipulse = app.ipulse-100;
		this.peak(2, 2);

		//this.spectrum.layer.clear();
		//context.globalAlpha = 0.9;

		//context.fillStyle = this.gdBack;
		//context.fillStyle = "rgba(0, 0, 0, 0.0)";
		//context.fillRect(0, 0, width, height);

		context.clearRect(0, 0, width, height);

		//context.globalAlpha = 0.9;
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

			this.bars[i].y = Math.max(height-pixelSize, 0);
			this.bars[i].h = Math.min(this.bars[i].h, height);

			context.fillRect(this.bars[i].x, this.bars[i].y, this.bars[i].w, this.bars[i].h);
			//context.lineTo(this.bars[i].x, this.bars[i].y);
		}
		context.lineTo(width, height);
		
		/*
		context.lineTo(0, height);
		context.stroke();
		*/

		context.fill();

		context.strokeStyle = "rgba(0, 0, 0, 0.4)";
		context.lineWidth = 3;
		for (var y=0; y<height; y+=4){
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


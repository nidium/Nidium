/* --------------------------------------------------------------------------- *
 * NATiVE DSP DEMO                                         (c) 2013 Stight.com * 
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
	dftBuffer : new Float64Array(256),

	start : function(url){
		this.createNodes();
		this.connectNodes();
		this.load(url);
		this.setProcessor();
		this.attachListeners();
	},

	createNodes : function(){
		this.dsp = new Audio(4096, 2, 44100);
		this.source = this.dsp.createNode("source", 0, 2);
		this.gain = this.dsp.createNode("gain", 2, 2);
		this.processor = this.dsp.createNode("custom", 2, 2);
		this.target = this.dsp.createNode("target", 2, 0);
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

		/* Threaded Audio Processor */
		this.processor.onbuffer = function(ev, scope){
			var bufferL = ev.data[0],
				bufferR = ev.data[1],
				size = bufferL.length,
				gain = this.get("gain");

			this.send({
				bufferL : bufferL,
				bufferR : bufferR
			});

			for (var i=0; i<size; i++){
				bufferL[i] *= gain;
				bufferR[i] *= gain;
			}
		}

		this.processor.onmessage = function(e){
			self.iqDFT(e.message.bufferL, e.message.bufferR);
		}

		this.processor.set("gain", 0.5);
	},

	/*
	 * UltraFast Magic DFT by Iñigo Quílez
	 * JS implementation by Vincent Fontaine
	 * http://www.iquilezles.org/www/articles/sincos/sincos.htm
	 */
	iqDFT : function(bufferL, bufferR){
		var bufferSize = bufferL.length, //256
			len = bufferSize >> 1, // 128
			angularNormalisation = k2PI/(len); // len

		// len = 128 for 2048 Audio Bytes Buffer

		for(var i=0; i<len>>1; i++){
			var wi = i * angularNormalisation,
				sii = Math.sin(wi),	coi = Math.cos(wi),
				co = 1.0, si = 0.0,	acco = 0.0, acsi = 0.0;

			for(var j=0; j<len; j++){
				var f = bufferL[j],
					oco = co;

				acco += co*f; co = co*coi -  si*sii;
				acsi += si*f; si = si*coi + oco*sii;
			}
			this.dftBuffer[i] = Math.sqrt(acco*acco + acsi*acsi) * 1/64;
		}
		Spectral.ondata();
	},

	connectNodes : function(){
		this.dsp.connect(this.source.output(0), this.gain.input(0));
		this.dsp.connect(this.source.output(1), this.gain.input(1));

		this.dsp.connect(this.gain.output(0), this.processor.input(0));
		this.dsp.connect(this.gain.output(1), this.processor.input(1));

		this.dsp.connect(this.processor.output(0), this.target.input(0));
		this.dsp.connect(this.processor.output(1), this.target.input(1));
	},

	attachListeners : function(){
		this.source.onplay = function(){
			echo("Playing");
		};

		this.source.onstop = function(){
			echo("Stopped");
		};

		this.source.onerror = function(err){
			echo("Error : " + err);
		};

		this.source.onbuffering = function(value){
			echo("Buffering : " + value);
		};
	}
};


var Spectral = {
 	loga : false,

 	UIBars : [],
 	bars : [],

 	nbars : 64,
	barSize : 200,

	cw : 1000,
	ch : 200,

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
			opacity : 0.9
		});

		this.spectrum.addEventListener("drag", function(e){
			this.left += e.xrel;
			this.top += e.yrel;
		});

		this.spectrum.slider = this.spectrum.add("UISliderController", {
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
			max : 5,
			value : 0
		});

		this.spectrum.slider.addEventListener("change", function(e){
			app.processor.set("gain", this.value);
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
			this.UIBars[i] = this.spectrum.add("UIView", {
				left : 1 + i * this.barW,
				top : this.barY,
				width : this.barW - 2,
				height : this.barSize,
				background : this.gdSpectrum
			});
		}

		this.makeBars();

		Native.layout.drawHook = function(){
			self.draw();
		};

	},

	clearBuffer : function(){
		for (var i=0; i<this.nbars; i++){
			app.dftBuffer[i] = 0;
		}
	},

	ondata : function(){
		for (var i=0 ; i<this.nbars ; i++){
			var value = app.dftBuffer[i],
				pixelSize = value*this.barSize;

			this.UIBars[i].height = pixelSize;
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

			//context.fillRect(this.bars[i].x, this.bars[i].y, this.bars[i].w, this.bars[i].h);
			context.lineTo(this.bars[i].x, this.bars[i].y);
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
app.start("dire.mp3");










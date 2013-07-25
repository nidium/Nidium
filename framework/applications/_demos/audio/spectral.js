/* -------------------------- */
/* Native (@) 2012 Stight.com */
/* -------------------------- */

/* --- DEMO APP : RealTime Spectral Analyser --------------------------------------------------------- */

var main = new Application({animation:false});

var Spectral = {

 	bars : [],
 	nbars: 64,
	barSize : 190,

	cw : canvas.width,
	ch : canvas.height,

	barW : 0,
	barY : 0,

	dftBuffer : [],

	init : function(buffer){
		this.barW = this.cw / this.nbars;
		this.barY = this.ch - this.barSize;

		var self = this,
			gdBack = canvas.createLinearGradient(0, 0, 0, this.ch),
			gdSpectrum = canvas.createLinearGradient(0, this.barY, 0, this.ch);

		gdBack.addColorStop(0.00,'#444444');
		gdBack.addColorStop(1.00,'#000000');

		gdSpectrum.addColorStop(0.00,'#ff0000');
		gdSpectrum.addColorStop(0.25,'#ffff00');
		gdSpectrum.addColorStop(0.50,'#00ff00');
		gdSpectrum.addColorStop(1.00,'#0000ff');

		main.background = gdBack;

		for (let i=0 ; i<this.nbars ; i++){
			this.bars[i] = main.add("UIView", {
				x : 1 + i * this.barW,
				y : this.barY,
				w : this.barW - 2,
				h : this.barSize,
				background : gdSpectrum
			});

		}

		canvas.requestAnimationFrame(function(){
			Native.layout.draw();
			self.drawLines();
		});

	},

	drawLines : function(){
		canvas.strokeStyle = "#000000";
		for (let y=0; y<this.ch; y+=4){
			canvas.beginPath();
			canvas.moveTo(0, y);
			canvas.lineTo(this.cw, y);
			canvas.stroke();
		}
	},

	fill : function(){
		for (let i=0; i<128; i++){
			var value = Math.round( Math.random()*65535);
			this.dftBuffer[i] = value;
		}
	},

	display : function(){
		for (let i=0 ; i<this.nbars ; i++){
			let value = this.dftBuffer[i],
				pixelSize = value*this.barSize/65535;

			this.bars[i].h = pixelSize;
			this.bars[i].y = this.ch-pixelSize;
		}
	},

	start : function(){
		var self = this,
			t = setTimeout(function(){
				self.display();
			}, 20, true, true);

	}

};

Spectral.init();
Spectral.fill();
Spectral.start();

var dft = [],
	buffer = [];

var iqDFT12 = function(dft, buffer){
	
	for(let i=0; i<128; i++) {
		let wi = i * (2*3.1415927/4096),
			sii = Math.sin(wi),
			coi = Math.cos(wi),

			co = 1.0,
			si = 0.0,
			acco = 0.0,
			acsi = 0.0;

		for(let j=0; j<4096; j++){
			let f = buffer[2*j+0] + buffer[2*j+1],
				oco = co;

			acco += co*f; co = co*coi -  si*sii;
			acsi += si*f; si = si*coi + oco*sii;
		}
		dft[i] = Math.sqrt(acco*acco+acsi*acsi)*(1/32767);
	}

};

iqDFT12(dft, buffer);


var int16 = function(buffer){
	return new int16View(buffer);
};

canvas.onAudio = function(buffer, len){
	var out = (int16) (buffer);

	for (var i=0; i<int16View.length; i++) {
		int16View[i] *= 0.5;
	}
	return out;
}



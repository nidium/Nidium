/* --------------------------------------------------------------------------- *
 * AUDIO DSP LIB                                           (c) 2013 nidium.com * 
 * --------------------------------------------------------------------------- * 
 * Version:     0.1.3                                                          *
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

/* -------------------------------------------------------------------------- */ 
/* UltraFast Magic DFT by Iñigo Quílez                                        */
/* ported from http://www.iquilezles.org/www/articles/sincos/sincos.htm       */
/* -------------------------------------------------------------------------- */ 

Audio.iqDFT = function(bufferL, bufferR, dftBuffer){
	var dftSize = dftBuffer.byteLength,
		bufferSize = bufferL.length,
		n = 2.0 * Math.PI/(bufferSize),
		s = 1.0/256.0; // 32767 in original Iñigo implementation

	for(var i=0; i<dftSize; i++){
		var wi = i * n,
			sii = Math.sin(wi),	coi = Math.cos(wi),
			co = 1.0, si = 0.0,	acco = 0.0, acsi = 0.0;

		for(var j=0; j<bufferSize; j++){
			var f = bufferL[j] + bufferR[j],
				oco = co;

			acco += co*f; co = co*coi -  si*sii;
			acsi += si*f; si = si*coi + oco*sii;
		}
		dftBuffer[i] = Math.sqrt(acco*acco + acsi*acsi) * s;
	}
};

/* -------------------------------------------------------------------------- */ 
/* Keep the lib in a function (Nidium Audio operates in its own thread)       */
/* -------------------------------------------------------------------------- */ 

Audio.lib = function(){
	var scope = this,
		abs = Math.abs,
		sin = Math.sin,
		cos = Math.cos,
		pow = Math.pow,
		exp = Math.exp,
		log = Math.log,
		sqrt = Math.sqrt,
		min = Math.min,
		max = Math.max,
		π = Math.PI,
		π2 = 2.0*π;

	var quadIn = function (t, b, c, d){
		return c*(t/=d)*t + b;
	};

	var quadOut = function (t, b, c, d){
		return -c *(t/=d)*(t-2) + b;
	};

	var quintIn = function (t, b, c, d){
		return c*(t/=d)*t*t*t*t + b;
	};

	var quintOut = function (t, b, c, d){
		return c*((t=t/d-1)*t*t*t*t + 1) + b;
	};

	// Checks if a number is a power of two
	var isPow2 = function(v){
		return !(v & (v-1)) && (!!v);
	};

	// Magnitude to decibels                                                      */
	var mag2db = function(magnitude){
		return 20.0*log(max(magnitude, pow(10.0, -6)));
	};

	// Lookup table for converting midi note to frequency
	var mtofnote = [
		0, 8.661957, 9.177024, 9.722718, 10.3, 10.913383, 11.562325, 12.25,
		12.978271, 13.75, 14.567617, 15.433853, 16.351599, 17.323914, 18.354048,
		19.445436, 20.601723, 21.826765, 23.124651, 24.5, 25.956543, 27.5,
		29.135235, 30.867706, 32.703197, 34.647827, 36.708096, 38.890873,
		41.203445, 43.65353, 46.249302, 49., 51.913086, 55., 58.27047,
		61.735413, 65.406395, 69.295654, 73.416191, 77.781746, 82.406891,
		87.30706, 92.498604, 97.998856, 103.826172, 110., 116.540939,
		123.470825, 130.81279, 138.591309, 146.832382, 155.563492, 164.813782,
		174.61412, 184.997208, 195.997711, 207.652344, 220., 233.081879,
		246.94165, 261.62558, 277.182617,293.664764, 311.126984, 329.627563,
		349.228241, 369.994415, 391.995422, 415.304688, 440., 466.163757,
		493.883301, 523.25116, 554.365234, 587.329529, 622.253967, 659.255127,
		698.456482, 739.988831, 783.990845, 830.609375, 880., 932.327515,
		987.766602, 1046.502319, 1108.730469, 1174.659058, 1244.507935,
		1318.510254, 1396.912964, 1479.977661, 1567.981689, 1661.21875,
		1760., 1864.655029, 1975.533203, 2093.004639, 2217.460938, 2349.318115,
		2489.015869, 2637.020508, 2793.825928, 2959.955322, 3135.963379,
		3322.4375, 3520., 3729.31, 3951.066406, 4186.009277, 4434.921875,
		4698.63623, 4978.031738, 5274.041016, 5587.651855, 5919.910645,
		6271.926758, 6644.875, 7040., 7458.620117, 7902.132812, 8372.018555,
		8869.84375, 9397.272461, 9956.063477, 10548.082031, 11175.303711,
		11839.821289, 12543.853516, 13289.75
	];


	/* ---------------------------------------------------------------------- */ 
	/* Clipper                                                                */
	/* ---------------------------------------------------------------------- */ 

	scope.clip = function(x){
		return max(min(x, 1), -1);
	};

	/* +----------+---------------------------------------------------------- */ 
	/* | Envelope | Decay                                                     */
	/* +----------+---------------------------------------------------------- */ 

	scope.Decay = function(duration){
		this.decay = 0.999 + duration * 0.0009;
		this.val = 0;
	};

	scope.Decay.prototype = {
		process : function(trig){
			this.val = trig ? 1 : this.val * this.decay;
			return this.val;
		}
	};

	/* +----------+---------------------------------------------------------- */ 
	/* | Envelope | ADSR                                                      */
	/* +----------+---------------------------------------------------------- */ 
	/*                           +                                            */
	/*   quadOut        .        .                                            */
	/*             .             |  .    Linear                               */
	/*         .                      .                                       */
	/*      .                    |      .                                     */
	/*    .                               .                                   */
	/*   .                       |           + . . . . . +                    */
	/*  .                                                  .     quadOut      */
	/* .                         |           |           |   .                */
	/* .                                                        .             */
	/* .                         |           |           |           .        */
	/* +-------------------------+-----------+-----------+----------------+-- */
	/* |         ATTACK          |   DECAY   |  SUSTAIN  |    RELEASE     |   */
	/* +-------------------------+-----------+-----------+----------------+-- */


	scope.ADSR = function(sampleRate){
		this.sampleRate = sampleRate || 44100;
		this.noteon = false;

		this.A = 0;
		this.D = 0;
		this.S = 1;
		this.R = 0;

		this.time = 0;
		this.volume = 0;
	};

	scope.ADSR.prototype = {
		update : function(trigger, attack, decay, sustain, release){
			var smp = this.sampleRate * 0.001;
			this.A = attack*smp;
			this.D = decay*smp;
			this.S = sustain;
			this.R = release*smp;

			if (this.noteon != trigger) {
				this.time = 0;
				this.noteon = trigger;
			}
		},

		process : function(){
			var v = 1,
				A = this.A,
				D = this.D,
				S = this.S,
				R = this.R,
				T = this.time++,
				Z = T-A;
	
			if (this.noteon){
				if (Z < 0){
					/* Attack */
					// this.volume = T/A; // Linear
					this.volume = quadIn(T, 0, 1, A);
				} else if (Z < D){
					/* Decay */
					this.volume = 1 + (S-1)*Z/D;
				} else {
					/* Sustain */
					this.volume = S;
				}
				v = this.volume;
			} else {
				/* Release */
				// v = (T-R)<0 ? this.volume * (1 - T/R) : 0; // Linear
				v = (T-R)<0 ? this.volume - quadOut(T, 0, this.volume, R) : 0;
			}
			
			return v;
		}
	};


	/* +----+---------------------------------------------------------------- */ 
	/* | FX | Distorsion                                                      */
	/* +----+---------------------------------------------------------------- */ 

	scope.Distorsion = function(){
		this.gain = 0.0;
	};

	scope.Distorsion.prototype = {
		update : function(gain){
			this.gain = gain;
		},

		process : function(input){
			var out = (1.0+this.gain) * input/(1.0+this.gain*abs(input));
			return max(min(out, 1), -1);
		}
	};

	/* +--------+------------------------------------------------------------ */ 
	/* | Filter | Resonant Filter                                             */
	/* +--------+------------------------------------------------------------ */ 

	scope.ResonantFilter = function(type, sampleRate){
		this.type = type;
		this.sampleRate = sampleRate;

		this.f = new Float64Array(4); // laggy
		this.f[0] = 0.0; // Low Pass
		this.f[1] = 0.0; // High Pass
		this.f[2] = 0.0; // Band Pass
		this.f[3] = 0.0; // Band Reject
	};

	scope.ResonantFilter.prototype = {
		update : function(cutoff, resonance){
			this.cutoff = cutoff || 0;
			this.resonance = resonance || 0;
			
			this.freq = 2 * sin(π * min(0.25, cutoff/(this.sampleRate*2)));

			this.damp = min(
				2 * (1 - pow(resonance, 0.25)),
				min(2, 2/this.freq - this.freq * 0.5)
			) || 0;
		},

		process : function(input){
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
		}
	};

	/* +--------+------------------------------------------------------------ */ 
	/* | Filter | Moog Filter                                                 */
	/* +--------+------------------------------------------------------------ */ 
	/* source : http://www.musicdsp.org/showArchiveComment.php?ArchiveID=26   */
	/* ---------------------------------------------------------------------- */ 

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

	scope.MoogFilter.prototype = {
		update : function(cutoff, resonance){
			this.cutoff = cutoff || 0;
			this.resonance = resonance || 0;
		},

		process : function(input){
			var f = this.cutoff * 1.16,
				f1 = 1 - f,
				ff = f * f;

			var fb = this.resonance * (1.0 - 0.15 * ff);

			input -= this.out4 * fb;
			input *= 0.35013 * (ff*ff);

			this.out1 = input + 0.3 * this.in1 + f1 * this.out1; // Pole 1
			this.in1  = input;

			this.out2 = this.out1 + 0.3 * this.in2 + f1 * this.out2;  // Pole 2
			this.in2  = this.out1;

			this.out3 = this.out2 + 0.3 * this.in3 + f1 * this.out3;  // Pole 3
			this.in3  = this.out2;

			this.out4 = this.out3 + 0.3 * this.in4 + f1 * this.out4;  // Pole 4
			this.in4  = this.out3;

			return this.out4;
		}
	};


	/* +--------+------------------------------------------------------------ */ 
	/* | Filter | Comb                                                        */
	/* +--------+------------------------------------------------------------ */ 

	scope.CombFilter = function(size){
		this.size = size;
		this.buffer = new Float64Array(size);

		this.feedback = 0.1;
		this.store = 0;
		this.damp1 = 0;
		this.damp2 = 0;
		this.idx = 0;

		this.reset();
	};

	scope.CombFilter.prototype = {
		reset : function(){
			for (var i=0; i<this.size; i++){
				this.buffer[i] = 0;
			}
		},

		update : function(damp, feedback){
			this.damp1 = damp || 0;
			this.damp2 = 1-damp || 0;
			this.feedback = feedback;
		},

		process : function(input){
			var output = this.buffer[this.idx];
			this.store = (output*this.damp2) + (this.store*this.damp1);
			this.buffer[this.idx] = input + (this.store*this.feedback);
			if (++this.idx >= this.size) this.idx = 0;
			return output;
		}
	};

	/* +--------+------------------------------------------------------------ */ 
	/* | Filter | AllPass                                                     */
	/* +--------+------------------------------------------------------------ */ 

	scope.AllPassFilter = function(size){
		this.size = size;
		this.buffer = new Float64Array(size);

		this.feedback = 0.5;
		this.idx = 0;

		this.reset();
	};

	scope.AllPassFilter.prototype = {
		reset : function(){
			for (var i=0; i<this.size; i++){
				this.buffer[i] = 0;
			}
		},

		update : function(feedback){
			this.feedback = feedback;
			this.reset();
		},

		process : function(input){
			var bufout = this.buffer[this.idx],
				output = bufout - input;

			this.buffer[this.idx] = input + (bufout*this.feedback);
			if (++this.idx>=this.size) this.idx = 0;
			return output;
		}
	};

	/* +----+---------------------------------------------------------------- */ 
	/* | FX | Reverb (JS implementation of FreeVerb)                          */
	/* +----+---------------------------------------------------------------- */ 
	/* source : https://ccrma.stanford.edu/~jos/pasp/Freeverb.html            */
	/* ---------------------------------------------------------------------- */ 

	/*
		Note: The original Schroeder reverb (Freeverb) sounds a bit oldish and
		metallic.

		The following implementation use 10 combfilters instead of 8,
		and 5 allpass filters in serie instead of 4.

		The original coefs also have been tripled for more realism when dealing
		with "large room".
		This is one of the tricks used when I made GlaceVerb 10 years ago.
	*/

	scope.Reverb = function(spread){
		var cbTunings = [1116, 1188, 1277, 1356, 1422, 1491, 1557, 1617, 1850, 2207],
			apTunings = [225, 556, 441, 341, 249];

		this.wet = 0.10;
		this.dry = 1.0;
		this.roomsize = 0.6;
		this.damp = 0.1;
		this.feedback = 0.5;

		this.combs = [];
		this.allpasses = [];

		for (var i=0; i<cbTunings.length; i++) {
			this.combs[i] = new scope.CombFilter(3*cbTunings[i]+spread);
			this.combs[i].damp = this.damp;
			this.combs[i].feedback = this.roomsize;
		}

		for (var i=0; i<apTunings.length; i++) {
			this.allpasses[i] = new scope.AllPassFilter(3*apTunings[i]+spread);
			this.allpasses[i].feedback = this.feedback;
		}

		this.update();
	};

	scope.Reverb.prototype = {
		update : function(wet, dry, roomsize, damp, feedback){
			this.wet = wet;
			this.dry = dry;
			this.roomsize = roomsize;
			this.damp = damp;
			this.feedback = feedback;

			for (var i=0; i<this.combs.length; i++) {
				this.combs[i].update(damp, roomsize);
			}
			for (var i=0; i<this.allpasses.length; i++) {
				this.allpasses[i].feedback = feedback;
			}
		},

		process : function(input){
			var output = 0;
			
			/* Parallel Comb Filters */
			for (var i=0; i<this.combs.length; i++){
				output += this.combs[i].process(input);
			}
			
			/* Send the signal to the serial All Pass Filters */
			for (var i=0; i<this.allpasses.length; i++){
				output = this.allpasses[i].process(output);
			}

			return input*this.dry + 0.02*output*this.wet;
		}
	};

	/* --- STEREO GLACEVERB ------------------------------------------------- */

	scope.GlaceVerb = function(){
		this.L = new scope.Reverb(-380);
		this.R = new scope.Reverb(+380);
	};

	scope.GlaceVerb.prototype = {
		update : function(wet, dry, roomsize, damp, feedback){
			this.L.update(wet, dry, roomsize, damp, feedback);
			this.R.update(wet, dry, roomsize, damp, feedback);
		},

		process : function(inL, inR){
			return [
				this.L.process(inL),
				this.R.process(inL)
			];
		}
	};

	/* ---------------------------------------------------------------------- */ 
	/* Stereo Width (2 in / 2 out)                                            */
	/* ---------------------------------------------------------------------- */ 

	scope.StereoWidth = function(){
		this.width = 0;
	};

	scope.StereoWidth.prototype = {
		update : function(width){
			this.width = width;
		},

		process : function(inL, inR){
			var scale = this.width * 0.5,
				m = (inL + inR) * 0.5,
				s = (inR - inL) * scale;

			return [m-s, m+s];
		}
	};

	/* ---------------------------------------------------------------------- */ 
	/* Simple Compressor                                                      */
	/* ---------------------------------------------------------------------- */ 

	scope.Compressor = function(){
		this.invScale = 1.0;
		this.gain = 0.5;
	};
	scope.Compressor.prototype = {
		update : function(scale, gain){
			this.invScale = 1/max(scale, 0.5);
			this.gain = isNaN(gain) ? 0.5 : gain;
		},

		process : function(input){
			var out = input * this.invScale;
			return (1+this.gain) * out - this.gain * out * out * out;
		}
	};

	/* ---------------------------------------------------------------------- */ 
	/* Normalizer                                                             */
	/* ---------------------------------------------------------------------- */ 

	scope.Normalizer = function(){
		this.pos = 0.0;
		this.max = 0.0;
		this.follower = 0.0;
	};

	scope.Normalizer.prototype = {
		process : function(input){
			this.max = (input>this.max) ? input : this.max * 0.9998;
			this.follower = this.follower * 0.9998 + this.max * 0.0002;
			input *= 1.0/this.follower;
			return input;
		}
	};

	/* +------------+-------------------------------------------------------- */ 
	/* | Generators | Noise                                                   */
	/* +------------+-------------------------------------------------------- */ 

	scope.Noise = function(sampleRate){
		this.sampleRate	= isNaN(sampleRate) ? sampleRate : 44100;

		this.b0 = 0; this.b1 = 0;
		this.b2 = 0; this.b3 = 0;
		this.b4 = 0; this.b5 = 0;

		this.c1 = null; this.c2 = null;
		this.c3 = null; this.c4 = null;

		this.q = 15;
		this.q0 = null;
		this.q1 = null;

		this.brownQ = 0;

		this.c1 = (1 << this.q) - 1;
		this.c2 = (~~(this.c1 /3)) + 1;
		this.c3 = 1 / this.c1;
		this.c1 = this.c2 * 6;
		this.c4 = 3 * (this.c2 - 1);
		this.q0 = exp(-200 * π / this.sampleRate);
		this.q1 = 1 - this.q0;
	};

	scope.Noise.prototype = {
		white : function(){
			var r = Math.random();
			return (r * this.c1 - this.c4) * this.c3;
		},

		pink : function(){
			var	w	= this.white();
			this.b0 = 0.997 * this.b0 + 0.029591 * w;
			this.b1 = 0.985 * this.b1 + 0.032534 * w;
			this.b2 = 0.950 * this.b2 + 0.048056 * w;
			this.b3 = 0.850 * this.b3 + 0.090579 * w;
			this.b4 = 0.620 * this.b4 + 0.108990 * w;
			this.b5 = 0.250 * this.b5 + 0.255784 * w;
			return 0.55 * (
				this.b0 + this.b1 + this.b2 + this.b3 + this.b4 + this.b5
			);
		},

		brown : function(){
			var	w = this.white();
			this.brownQ	= (this.q1 * w + this.q0 * this.brownQ);
			return 6.2 * this.brownQ;
		}
	};

	/* +------------+-------------------------------------------------------- */ 
	/* | Generators | Low Frequency Oscillator                                */
	/* +------------+-------------------------------------------------------- */ 

	scope.LFO = function(sampleRate, shape, freq){
		this.frequency	= isNaN(freq) ? 440 : freq;
		this.sampleRate = sampleRate;

		this.phaseOffset = 0;
		this.pulseWidth = 0.5;
		this.fm = 0.00;
		this.shape = shape || 'sine';
		this.phase = 0;
		this._p = 0;
	};

	scope.LFO.prototype = {
		generate : function(){
			var	f = +this.frequency,
				pw = this.pulseWidth,
				p = this.phase;

			f += f*this.fm;

			this.phase = (p + f / this.sampleRate / 2) % 1;
			p = (this.phase + this.phaseOffset) % 1;
			this._p = p < pw ? p / pw : (p-pw) / (1-pw);
		},

		sine : function(){
			return sin(this._p * π2);
		},

		triangle : function(){
			return this._p < 0.5 ? 4*this._p - 1 : 3 - 4*this._p;
		},

		square : function(){
			return this._p < 0.5 ? -1 : 1;
		},

		saw : function(){
			return 1 - this._p * 2;
		},

		invsaw : function(){
			return this._p * 2 - 1;
		},

		pulse : function(){
			return 	this._p < 0.5 ?
						this._p < 0.25 ?
							this._p*8 - 1 : 1 - (this._p-0.25) * 8 : -1;
		},

		getSample : function(){
			return this[this.shape]();
		}
	};

	return scope;
};
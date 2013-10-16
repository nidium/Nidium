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
	// integer
	var i, j,
		dftSize = dftBuffer.length, // 128 values (64bits each) 
		samples = bufferL.length; // 256 values (64bits each)

	// float
	var	f, wi, sii, coi, oco, co, si, real, imag,
		n = 2.0 * Math.PI/(samples),
		s = 1.0/128.0; // 32767 in original Iñigo implementation

	for (i=0; i<dftSize; i++){
		wi = i * n;
		sii = Math.sin(wi);
		coi = Math.cos(wi);
		co = 1.0;	si = 0.0;
		real = 0.0;	imag = 0.0;

		for (j=0; j<samples; j++){
			oco = co;
			f = bufferL[j] + bufferR[j];
			real += co*f; co = co*coi -  si*sii;
			imag += si*f; si = si*coi + oco*sii;
		}
		// compute the magnitude of the complex number
		dftBuffer[i] = Math.sqrt(real*real + imag*imag) * s;
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
		atan2 = Math.atan2,
		round = Math.round,
		ceil = Math.ceil,
		floor = Math.floor;

	const π = Math.PI,
	      π2 = 2.0*π;

	/* ---------------------------------------------------------------------- */ 
	/* Interpolation Formulas                                                 */
	/* ---------------------------------------------------------------------- */ 

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

	/* ---------------------------------------------------------------------- */ 
	/* Bitwise Tricks and Helpers                                             */
	/* ---------------------------------------------------------------------- */ 

	var sign = function(x){
		return (x === 0) ? 1 : abs(x) / x;
	};

	// Checks if a number is a power of two
	var isPow2 = function(v){
		return !(v & (v-1)) && (!!v);
	};

	// absolute value of integer
	var iabs = function(v) {
		var mask = v >> 31;
		return (v ^ mask) - mask;
	};

	// minimum of integers x and y
	var imin = function(x, y) {
		return y ^ ((x ^ y) & -(x < y));
	};

	// maximum of integers x and y
	var imax = function(x, y) {
		return x ^ ((x ^ y) & -(x < y));
	};

	// log base 2 of v
	var log2 = function(v) {
		var r, shift;
		r =     (v > 0xFFFF) << 4; v >>>= r;
		shift = (v > 0xFF  ) << 3; v >>>= shift; r |= shift;
		shift = (v > 0xF   ) << 2; v >>>= shift; r |= shift;
		shift = (v > 0x3   ) << 1; v >>>= shift; r |= shift;
		return r | (v >> 1);
	};

	// log base 10 of v
	var log10 = function(v) {
		return (v >= 1000000000) ? 9 : (v >= 100000000) ? 8 : (v >= 10000000) ? 7 :
		       (v >= 1000000) ? 6 : (v >= 100000) ? 5 : (v >= 10000) ? 4 :
		       (v >= 1000) ? 3 : (v >= 100) ? 2 : (v >= 10) ? 1 : 0;
	};

	// Magnitude to decibels
	var mag2db = function(magnitude){
		return 20.0*log(max(magnitude, pow(10.0, -6)));
	};

	// Decibels to magnitude
	var db2mag = function(db){
		return max(0, round(100 * pow(2, db / 6)) / 100);
	};

	/* ---------------------------------------------------------------------- */ 
	/* Lookup table for converting midi note to frequency                     */
	/* ---------------------------------------------------------------------- */ 

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
			return scope.clip(out);
		}
	};

	/* +----+---------------------------------------------------------------- */ 
	/* | FX | Overdrive                                                       */
	/* +----+---------------------------------------------------------------- */ 

	scope.Overdrive = function(sampleBufferLength){
		this.gain = 0.0;
		this.size = sampleBufferLength;
	};

	scope.Overdrive.prototype = {
		update : function(gain){
			this.gain = gain;
		},

		process : function(input, phase){
			var amount = min(this.gain, 0.9999),
				k = 2 * amount / (1 - amount),
				x = 2*phase / this.size - 1,
				buf = (1+k) * x / (1 + k * abs(x));

			return scope.clip(
				0.01*input*buf*this.gain + 
				(1.0+this.gain) * input/(1.0+this.gain*abs(input))
			);
		}
	};

	/* +--------+------------------------------------------------------------ */ 
	/* | Filter | Resonant Filter                                             */
	/* +--------+------------------------------------------------------------ */ 

	scope.ResonantFilter = function(type, sampleRate){
		this.type = type;
		this.sampleRate = sampleRate;

		this.f = new Float64Array(4);
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
				this.R.process(inR)
			];
		}
	};


	/* +----+---------------------------------------------------------------- */ 
	/* | FX | Flanger                                                         */
	/* +----+---------------------------------------------------------------- */ 

	scope.Flanger = function(sampleRate){
		this.phase = 0;
		this.offset = -100;
		this.index = 0;

		this.sampleRate = sampleRate || 44100;
		this.bufsize = 2*this.sampleRate;

		this.sin = new Float64Array(2048);
		this.buffer = new Float64Array(this.bufsize);

		for (var i=0; i<2048; i++){
			this.sin[i] = sin( π2*(i/2048) );
		}

		this.update(0.25, 0.5, 0.5, 0.1);
	};

	scope.Flanger.prototype = {
		update : function(rate, amplitude, amount, feedback){
			this.rate = rate || 0;
			this.amplitude = amplitude || 0;
			this.amount = amount || 0;
			this.feedback = feedback || 0;

			this.__incsin__ = this.rate / this.sampleRate;
			this.__incpre__ = 2048 * this.__incsin__;
			this.__am__ = this.amplitude * 100;
		},

		__modulation__ : function(){
			/* classical naive Math.sin implementation */
			this.phase += this.__incsin__;
			return sin(π2 * this.phase) * this.__am__;
		},

		modulation : function(){
			/* precomp sin table + linear interpolation */
			var p = this.phase,
				i = 0,
				f = 0,
				v1 = 0,
				v2 = 0;

			p += this.__incpre__;
			while (p>=2048) p -= 2048; // faster than p = p % 2048

			i = p | 0; // truncate to int
			f = p - i;
			
			//i = i & 1023;
			//j = i === 1023 ? 0 : i+1;

			v1 = this.sin[i];
			v2 = this.sin[(i+1) & 2047];

			this.phase = p;

			return ( v1 + f*(v2-v1) ) * this.__am__;
		},

		getDelaySample : function(phase){
			var	b = this.buffer,
				i = phase | 0,
				j = i>this.bufsize-2 ? 0 : i+1;
				f = phase - i;

			return b[i] + f*(b[j] - b[i]);
		},

		process : function(input){
			var delay = 0.0,
				s = this.bufsize,
				o = (this.offset++) % s,
				i = (this.index++) % s;

			o += this.modulation();

			/* Ring Buffer */
			if (o>s) {
				o -= s;
			} else if (o<0) {
				o += s;
			}

			delay = this.getDelaySample(o);
			this.buffer[i] = input + delay*this.feedback;

			return (1-this.amount) * input + this.amount * delay;
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

	/* ---------------------------------------------------------------------- */ 
	/* Fast Fourier Transform (Maybe the Fastest JavaScript implementation)   */
	/* ---------------------------------------------------------------------- */ 
	/* Description: this compute an in-place radix-2 complex-to-complex FFT   */
	/* ---------------------------------------------------------------------- */
	/* Original: http://paulbourke.net/miscellaneous/dft/                     */
	/* ---------------------------------------------------------------------- */
	/* x and y are the real and imaginary arrays of n points                  */
	/* dir =  1 ---> forward transform                                        */
	/* dir = -1 ---> reverse transform                                        */
	/* ---------------------------------------------------------------------- */
	/*

	-----------
	Forward FFT
	-----------
	                 N-1
	                 ---
	             1    \            - j k 2π n / N
	  X(n)  =   ---    >    x(k)  e                    = forward transform
	             N    /                                  n = 0..N-1
	                 ---
	                 k=0

	-----------
	Reverse FFT
	-----------
	                 N-1
	                 ---
	                 \              j k 2π n / N
	       X(n) =     >     x(k)  e                     = forward transform
	                 /                                   n = 0..N-1
	                 ---
	                 k=0
	*/

	scope.FFT = function(n){
		this.n = n;
		this.m = log(n)/log(2);
		if (n != (1<<this.m)) throw new Error("FFT length must be power of 2");
	};

	scope.FFT.prototype = {
		process : function(x, y, dir){
			var n = this.n, m = this.m;
			var i,i1,j,k,i2,l,l1,l2; // long
			var c1,c2,tx,ty,t1,t2,u1,u2,z; // double

			/* Do the bit reversal */
			j = 0;
			i2 = n >> 1;
			for (i=0;i<n-1;i++){
				if (i < j){
					tx = x[i];
					ty = y[i];
					x[i] = x[j];
					y[i] = y[j];
					x[j] = tx;
					y[j] = ty;
				}
				k = i2;
				while (k <= j){
					j -= k;
					k >>= 1;
				}
				j += k;
			}

			/* Compute the FFT */
			l2 = 1;
			c1 = -1.0;
			c2 = 0.0;
			for (l=0; l<m; l++){
				l1 = l2; l2 <<= 1;
				u1 = 1.0; u2 = 0.0;
				for (j=0; j<l1; j++){
					for (i=j; i<n; i+=l2){
						i1 = i + l1;
						t1 = u1*x[i1] - u2*y[i1];
						t2 = u1*y[i1] + u2*x[i1];
						x[i1] = x[i] - t1;
						y[i1] = y[i] - t2;
						x[i] += t1;
						y[i] += t2;
					}
					z =  u1 * c1 - u2 * c2;
					u2 = u1 * c2 + u2 * c1;
					u1 = z;
				}
				c2 = sqrt((1.0 - c1) / 2.0);
				c1 = sqrt((1.0 + c1) / 2.0);
				c2 = (-dir) * c2; // branch removed if (dir==1) c2 = -c2;
			}

			/* Scaling for forward transform */
			if (dir == 1) {
				for (i=0; i<n; i++){
					x[i] /= n;
					y[i] /= n;
				}
			}
		}
	};

	/* ---------------------------------------------------------------------- */ 
	/* In-place Radix-2 Real/Img Interleaved FFT + reverse FFT                */
	/* ---------------------------------------------------------------------- */ 
	/* Original C code: http://downloads.dspdimension.com/smbPitchShift.cpp   */
	/* Author: Stephan M. Bernsee <smb [AT] dspdimension [DOT] com>           */
	/* JS port: Vincent Fontaine                                              */
	/* ---------------------------------------------------------------------- */ 
	/* 
	Sign:         "value",  -1 is FFT, 1 is iFFT (inverse)
	fftFrameSize: must be a power of 2.

	buffer:       Fills buffer[0 ... 2*fftFrameSize-1] with the Fourier
	              transform of the time domain data in
	              buffer[0 ... 2*fftFrameSize-1]

	The FFT array takes	and returns the cosine and sine parts
	in an interleaved manner:
	buffer[0] = cosPart[0]
	buffer[1] = sinPart[0]

	It expects a complex input signal. When working with 'common' audio signals
	our input signal has to be passed as {in[0], 0., in[1], 0., in[2], 0., ...}
	In that case, the transform of the frequencies of interest
	is in buffer[0 ... fftFrameSize].
	*/

	scope.smbFFTJS = function(buffer, fftFrameSize, sign){
		// float
		var wr, wi, arg, tr, ti, ur, ui, temp, p1, p2,
			p1r, p1i, p2r, p2i, kr, ki;

		// long
		var i, bitm, j, le, le2, k,
			fftsize2 = 2*fftFrameSize,
			lg = (log(fftFrameSize)/log(2) + 0.5)|0;

		for (i=2; i<fftsize2-2; i+=2){
			for (bitm=2, j=0; bitm < fftsize2; bitm <<= 1){
				if (i & bitm) j++;
				j <<= 1;
			}
			if (i<j){
				p1 = i;
				p2 = j;

				temp = buffer[p1]; // temp = *p1;
				buffer[p1++] = buffer[p2]; // *(p1++) = *p2;
				buffer[p2++] = temp; // *(p2++) = temp;
				
				temp = buffer[p1]; //temp = *p1;
				buffer[p1] = buffer[p2]; // *p1 = *p2;
				buffer[p2] = temp; // *p2 = temp;
			}
		}

		for (k=0, le=2; k<lg; k++){
			le <<= 1; le2 = le>>1;
			ur = 1.0; ui = 0.0;
			arg = π2 / (le2>>1);
			wr = cos(arg);
			wi = sign * sin(arg);
			for (j=0; j<le2; j+=2){
				p1r = j; p1i = p1r+1;
				p2r = p1r+le2; p2i = p2r+1;
				for (i=j; i<fftsize2; i+=le){
					kr = buffer[p2r]; ki = buffer[p2i];
					tr = kr * ur - ki * ui;
					ti = kr * ui + ki * ur;
					buffer[p2r] = buffer[p1r] - tr;
					buffer[p2i] = buffer[p1i] - ti;
					buffer[p1r] += tr;
					buffer[p1i] += ti;
					p1r += le; p1i += le;
					p2r += le; p2i += le;
				}
				tr = ur*wr - ui*wi;
				ui = ur*wi + ui*wr;
				ur = tr;
			}
		}
	};

	/* ---------------------------------------------------------------------- */ 
	/* RealTime Pitch Shifting - JavaScript Implementation                    */
	/* ---------------------------------------------------------------------- */ 
	/* Pitch Shifting while maintaining duration using the STF Transform.     */
	/* ---------------------------------------------------------------------- */ 
	/* original C code: http://downloads.dspdimension.com/smbPitchShift.cpp   */
	/* Author: Stephan M. Bernsee <smb [AT] dspdimension [DOT] com>           */
	/* ---------------------------------------------------------------------- */ 
	/* DESCRIPTION: The routine takes a pitchShift factor value which         */
	/* is between 0.5 (one octave down) and 2.0 (one octave up).              */
	/* A value of exactly 1 does not change the pitch.                        */
	/*                                                                        */
	/* samples tells the routine the size of the indata sample buffer.        */
	/* indata is processed in-place.                                          */
	/*                                                                        */
	/* fftFrameSize defines the FFT frame size used for the processing.       */
	/* Typical values are 1024, 2048 and 4096. It may be any value            */
	/*  <= MAX_FRAME_LENGTH but it MUST be a power of 2.                      */
	/*                                                                        */
	/* osamp is the STFT oversampling factor which also determines the        */
	/* overlap between adjacent STFT frames. It should at least be 4 for      */
	/* moderate scaling ratios. A value of 32 is recommended for best         */
	/* quality.                                                               */
	/*                                                                        */
	/* sampleRate takes the sample rate for the signal in unit Hz             */
	/* (44100 for 44.1 kHz audio).                                            */
	/*                                                                        */
	/* The data passed to the routine in indata[] should be in the            */
	/* range [-1.0, 1.0], which is also the output range.                     */
	/* ---------------------------------------------------------------------- */ 
	/* WOL License (The Wide Open License)                                    */
	/* ---------------------------------------------------------------------- */ 
	/* Permission to use, copy, modify, distribute and sell this              */
	/* software and its documentation for any purpose is hereby granted       */
	/* without fee, provided that the above copyright notice and this         */
	/* license appear in all source copies. THIS SOFTWARE IS PROVIDED         */
	/* "AS IS" WITHOUT EXPRESS OR IMPLIED WARRANTY OF ANY KIND.               */
	/* See http://www.dspguru.com/wol.htm for more information.               */
	/* ---------------------------------------------------------------------- */ 


	scope.PitchShift = function(sampleRate, oversampling){
		const MAX_FRAME_LENGTH = 8192; //8192;

		this.oversampling = 4; //oversampling || 4;
		this.fftFrameSize = 1024;
		this.sampleRate = sampleRate || 44100;

		this.gRover = false;

		var Float = function(size){
			var buffer = new Float64Array(size);
			for (var i=0; i<size; i++){
				buffer[i] = 0.0;
			}
			return buffer;
		};

		/* input and output FIFO */
		this.gInFIFO = Float(MAX_FRAME_LENGTH);
		this.gOutFIFO = Float(MAX_FRAME_LENGTH);

		this.gLastPhase = Float(MAX_FRAME_LENGTH / 2 + 1);
		this.gSumPhase = Float(MAX_FRAME_LENGTH / 2 + 1);

		this.gFFTworksp = Float(2*MAX_FRAME_LENGTH);
		this.gOutputAccum = Float(2 * MAX_FRAME_LENGTH);

		/* Analysis Buffers */
		this.gAnaFreq = Float(MAX_FRAME_LENGTH);
		this.gAnaMagn = Float(MAX_FRAME_LENGTH);

		/* Synthesis Buffers */
		this.gSynFreq = Float(MAX_FRAME_LENGTH);
		this.gSynMagn = Float(MAX_FRAME_LENGTH);

		this.win = Float(this.fftFrameSize);

		// Hann Window */
		for (k=0; k<this.fftFrameSize; k++){
			this.win[k] = -0.5*cos(π2*k/this.fftFrameSize)+0.5;
		}
	};

	scope.PitchShift.prototype = {
		mDoubleCopy : function(dBuffer, dOffset, sBuffer, sOffset, size){
			var tmp = new Float64Array(size);
			for (var i=0; i<size; i++){
				tmp[i] = sBuffer[i+sOffset];
			}
			for (var i=0; i<size; i++){
				dBuffer[i+dOffset] = tmp[i];
			}
		},

		process : function(pitchShift, samples, buffer){
			function memset(buffer, value, size){
				for (var i=0; i<size; i++){
					buffer[i] = value;
				}
			}

			var osamp = this.oversampling,
				STFT = scope.smbFFTJS,
				fftFrameSize = this.fftFrameSize,
				fftFrameSize2 = fftFrameSize>>1,

				stepSize = fftFrameSize / osamp,
				freqPerBin = this.sampleRate / fftFrameSize,
				expct = π2 * stepSize / fftFrameSize,

				inFifoLatency = fftFrameSize - stepSize,
				j, k = 0,
				real, imag,
				magn, phase,
				tmp, qpd, index;

			if (this.gRover === false) {
				this.gRover = inFifoLatency;
			}

			for (j=0; j<samples; j++) {
				this.gInFIFO[this.gRover] = buffer[j];
				buffer[j] = this.gOutFIFO[this.gRover - inFifoLatency];
				this.gRover++;

				if (this.gRover >= fftFrameSize) {
					this.gRover = inFifoLatency;

					for (k=0; k<fftFrameSize; k++){
						this.gFFTworksp[2*k] = this.gInFIFO[k] * this.win[k];
						this.gFFTworksp[2*k+1] = 0.0;
					}

					STFT(this.gFFTworksp, fftFrameSize, -1);

					/* -- ANALYSIS ------------------------------------------ */

					for (k=0; k<=fftFrameSize2; k++){

						/* de-interlace FFT buffer */
						real = this.gFFTworksp[2*k];
						imag = this.gFFTworksp[2*k+1];

						/* compute magnitude and phase */
						magn = 2*sqrt(real*real + imag*imag);
						phase = atan2(imag, real);

						/* compute phase difference */
						tmp = phase - this.gLastPhase[k];
						this.gLastPhase[k] = phase;

						/* subtract expected phase difference */
						tmp -= k*expct;

						/* map delta phase into +/- Pi interval */
						qpd = tmp/π;
						qpd = (qpd>=0) ? qpd + qpd&1 : qpd - qpd&1;
						tmp -= π2*qpd;

						/* get frequency deviation from the +/- Pi interval */
						tmp *= osamp/π2;

						/* compute the k-th partials' true frequency */
						tmp = k*freqPerBin + tmp*freqPerBin;

						/* store magn and true frequency in analysis arrays */
						this.gAnaMagn[k] = magn;
						this.gAnaFreq[k] = tmp;
					}

					/* -- PROCESSING -------------------------------------------- */

					memset(this.gSynMagn, 0, fftFrameSize);
					memset(this.gSynFreq, 0, fftFrameSize);

					for (k=0; k<=fftFrameSize2; k++){ 
						index = (k*pitchShift)|0;
						if (index<=fftFrameSize2){ 
							this.gSynMagn[index] += this.gAnaMagn[k]; 
							this.gSynFreq[index] = this.gAnaFreq[k] * pitchShift;
						} 
					}

					/* -- SYNTHESIS ----------------------------------------- */

					for (k=0; k<=fftFrameSize2; k++){

						/* get magn and true frequency from synthesis arrays */
						magn = this.gSynMagn[k];
						tmp = this.gSynFreq[k];

						/* subtract bin mid frequency */
						tmp -= k*freqPerBin;

						/* get bin deviation from freq deviation */
						tmp /= freqPerBin;

						/* take osamp into account */
						tmp *= π2/osamp;

						/* add the overlap phase advance back in */
						tmp += k*expct;

						/* accumulate delta phase to get bin phase */
						this.gSumPhase[k] += tmp;
						phase = this.gSumPhase[k];

						/* get real and imag part and re-interleave */
						this.gFFTworksp[2*k+0] = magn*cos(phase);
						this.gFFTworksp[2*k+1] = magn*sin(phase);
					} 

					/* zero negative frequencies */
					for (k=fftFrameSize+2; k < 2*fftFrameSize; k++){
						this.gFFTworksp[k] = 0.0;
					}

					/* do inverse transform */
					STFT(this.gFFTworksp, fftFrameSize, 1);

					/* do windowing and add to output accumulator */ 
					for (k=0; k<fftFrameSize; k++){
						this.gOutputAccum[k] += 2*this.win[k] * 
									this.gFFTworksp[2*k]/(fftFrameSize2*osamp);
					}
					
					for (k=0; k<stepSize; k++){
						this.gOutFIFO[k] = this.gOutputAccum[k];
					}

					/* shift accumulator */
					//memmove(gOutputAccum, gOutputAccum+stepSize, fftFrameSize);
					this.mDoubleCopy(
						this.gOutputAccum, 0,
						this.gOutputAccum, stepSize,
						fftFrameSize
					);

					/* move input FIFO */
					for (k=0; k<inFifoLatency; k++){
						this.gInFIFO[k] = this.gInFIFO[k+stepSize];
					}

				}

			}

		}
	};

	return scope;
};

function __TEST_smbFFT__(fftSize){
	var scope = Audio.lib(),
		samples = 256,
		hann = new Float32Array(fftSize),
		tmpBuffer = new Float32Array(2*fftSize),
		fftBuffer = new Float32Array(2*fftSize); // interleaded real / imag

	// Precomputed Hann Window */
	for (var k=0; k<fftSize; k++){
		hann[k] = -0.5*Math.cos(2*Math.PI*k/fftSize)+0.5;
	}

	// Fill the interleaved FFT Buffer with random audio data (from -1 to 1)
	for (var k=0; k<fftSize; k++){
		fftBuffer[2*k+0] = 2*Math.random()-1 * hann[k]; // the real part
		fftBuffer[2*k+1] = 0.0; // the imag part
	}

	for (var k=0; k<fftSize; k++){
		tmpBuffer[k] = fftBuffer[k];
	}

	var resetFFTBuffer = function(){
		for (var k=0; k<fftSize; k++){
			fftBuffer[k] = tmpBuffer[k];
		}
	};

	var t = +new Date();
	for (var i=0; i<samples; i++){
		resetFFTBuffer();
		scope.smbFFTJS(fftBuffer, fftSize, -1);
		scope.smbFFTJS(fftBuffer, fftSize, 1);
	}
	console.log("smbFFT(JS):", (+new Date()) - t, "ms");


	var t = +new Date();
	for (var i=0; i<samples; i++){
		resetFFTBuffer();
		Audio.smbFFT(fftBuffer, fftSize, -1);
		Audio.smbFFT(fftBuffer, fftSize, 1);
	}
	console.log("smbFFT(C):",  (+new Date()) - t, "ms");

}

function __TEST_fntFFT__(fftSize){
	var scope = Audio.lib(),
		samples = 256,
		fftSize = 1024,
		tmpBuffer = new Float32Array(fftSize),
		x = new Float32Array(fftSize),
		y = new Float32Array(fftSize);

	for (var k=0; k<fftSize; k++){
		x[k] = 2*Math.random()-1; // the real part
		y[k] = 0.0; // the imag part
	}

	for (var k=0; k<fftSize; k++){
		tmpBuffer[k] = x[k];
	}

	var resetFFTBuffer = function(){
		for (var k=0; k<fftSize; k++){
			x[k] = tmpBuffer[k];
			y[k] = 0;
		}
	};

	var fft1 = new scope.FFT(fftSize);
	var fft2 = new scope.FFT(fftSize);

	var t = +new Date();
	for (var i=0; i<samples; i++){
		resetFFTBuffer();
		fft1.process(x, y,  1);
		fft2.process(x, y, -1);
	}
	console.log("fntFFT(JS)", (+new Date()) - t, "ms");
}


__TEST_smbFFT__(1024);
__TEST_fntFFT__(1024);
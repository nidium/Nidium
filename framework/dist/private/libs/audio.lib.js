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
/* http://www.iquilezles.org/www/articles/sincos/sincos.htm                   */
/* -------------------------------------------------------------------------- */ 

Audio.iqDFT = function(bufferL, bufferR, dftBuffer){
	var dftSize = dftBuffer.byteLength,
		bufferSize = bufferL.length,
		angularNormalisation = 2.0 * Math.PI/(bufferSize*4);

	for(var i=0; i<dftSize; i++){
		var wi = i * angularNormalisation,
			sii = Math.sin(wi),	coi = Math.cos(wi),
			co = 1.0, si = 0.0,	acco = 0.0, acsi = 0.0;

		for(var j=0; j<bufferSize; j++){
			var f = bufferL[j] + bufferR[j],
				oco = co;

			acco += co*f; co = co*coi -  si*sii;
			acsi += si*f; si = si*coi + oco*sii;
		}
		dftBuffer[i] = Math.sqrt(acco*acco + acsi*acsi) * 1/128;
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
		sqrt = Math.sqrt,
		min = Math.min,
		max = Math.max,
		π = Math.PI,
		π2 = 2.0*π;


	/* ---------------------------------------------------------------------- */ 
	/* Clipper                                                                */
	/* ---------------------------------------------------------------------- */ 

	scope.clip = function(x){
		return max(min(x, 1), -1);
	};

	/* ---------------------------------------------------------------------- */ 
	/* Decay                                                                  */
	/* ---------------------------------------------------------------------- */ 

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


	/* ---------------------------------------------------------------------- */ 
	/* Distorsion                                                             */
	/* ---------------------------------------------------------------------- */ 

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

	/* ---------------------------------------------------------------------- */ 
	/* Resonant Filter                                                        */
	/* ---------------------------------------------------------------------- */ 

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

	/* ---------------------------------------------------------------------- */ 
	/* Moog Filter                                                            */
	/* ---------------------------------------------------------------------- */ 
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
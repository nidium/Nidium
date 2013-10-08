/* --------------------------------------------------------------------------- *
 * AUDIO DSP LIB                                           (c) 2013 nidium.com * 
 * --------------------------------------------------------------------------- * 
 * Version:     0.1.0                                                          *
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

Audio.lib = function(){
	var scope = this,
		sin = Math.sin,
		cos = Math.cos,
		pow = Math.pow,
		sqrt = Math.sqrt,
		min = Math.min,
		max = Math.max,
		π = Math.PI;

	/* ---------------------------------------------------------------------- */ 

	scope.ResonantFilter = function(type, sampleRate){
		this.type = type;
		this.sampleRate = sampleRate;

		this.f = new Float32Array(4);
		this.f[0] = 0.0; // Low Pass
		this.f[1] = 0.0; // High Pass
		this.f[2] = 0.0; // Band Pass
		this.f[3] = 0.0; // Band Reject
	};

	scope.ResonantFilter.prototype = {
		update : function(cutoff, resonance){
			this.cutoff = cutoff;
			this.resonance = resonance;
			
			this.freq = 2 * sin(π * min(0.25, cutoff/(this.sampleRate*2)));

			this.damp = min(
				2 * (1 - pow(resonance, 0.25)),
				min(2, 2/this.freq - this.freq * 0.5)
			);
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
			this.cutoff = cutoff;
			this.resonance = resonance;
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

	scope.Compressor = function(){
		this.invScale = 1.0;
		this.gain = 0.5;
	};
	scope.Compressor.prototype = {
		update : function(scale, gain){
			this.invScale = 1/Math.max(scale, 0.5);
			this.gain = isNaN(gain) ? 0.5 : gain;
		},

		process : function(s){
			var out = s * this.invScale;
			return (1+this.gain) * out - this.gain * out * out * out;
		}
	};

	/* ---------------------------------------------------------------------- */ 
};
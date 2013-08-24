/* --------------------------------------------------------------------------- *
 * NATiVE MIXER DEMO                                       (c) 2013 Stight.com * 
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

var main = new Application();

var mixer = null,
	tracks = [];

var tx = [
	{file : "../media/song.mp3",	label : "Depress"},
	{file : "../media/song.mp3",	label : "Depress"},
	{file : "../media/song.mp3",	label : "Depress"}
];

for (var i=0; i<tx.length; i++){
	tracks[i] = AudioMixer.load(tx[i].file, function(data, k){
		this.volume(0.5);
		this.play();
	}, i);


	tracks[i].processor.onbuffer = function(ev, scope){
		var channels = ev.data,
			gain = this.get("gain");

		for (var n=0; n<channels.length; n++){
			var buffer = channels[n];
			for (var i=0; i<buffer.length; i++){
				buffer[i] *= gain;
			}
		}
	};

	tracks[i].processor.onmessage = function(e){
		//console.log(e.data);
	};

}

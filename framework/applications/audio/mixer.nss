/* --------------------------------------------------------------------------- *
 * NATiVE MIXER DEMO                                       (c) 2013 nidium.com * 
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

{
	"#audioMixerContainer" : {
		top : 300,
		left : 10,
		width : 1000,
		height : 350,
		background : "rgba(0, 0, 0, 0.20)",
		radius : 4
	},

	".slide" : {
		top : 0,
		width : 60,
		height : 336,
		background : "rgba(0, 0, 0, 0.30)",
		radius : 3
	},

	".panLabel" : {
		top : 0,
		label : "L                 R",
		fontSize : 9,
		color : "#888888"
	},

	".panSlider" : {
		top : 6,
		width : 32,
		height : 8,
		color : "#440000",
		min : -1,
		max : 1,
		value : 0,
		background : 'rgba(128, 128, 128, 0.2)',
		radius : 2
	},

	".levelSlider" : {
		top : 50,
		width : 22,
		height : 250,
		color : "black",
		splitColor : 'rgba(140, 140, 140, 0.3)',
		boxColor : 'rgba(255, 255, 255, 0.07)',
		progressBarColor : 'rgba(210, 255, 40, 1)',
		radius : 2,
		vertical : true,
		min : 0.0,
		max : 2,
		value : 0.5
	},

	".button" : {
		top : 20,
		height : 13,
		width : 14,
		autowidth : false,
		color : "#cccccc",
		background : "rgba(33, 0, 0, 0.6)",
		paddingLeft : 3,
		paddingRight : 3,
		radius : 6,
		fontSize : 9,
		fontFamily : "menlo",
		outlineColor : ""
	},

	".button:selected" : {
		textOffsetY : 0
	},

	".selected" : {
		background : "rgba(255, 0, 0, 0.6)",
	},

	".solo" : {
		left : 5,
		label : "S",
	},

	".mute" : {
		left : 44,
		label : "M",
		paddingLeft : 3
	},

	".grayed" : {
		left : 44,
		label : "M",
		paddingLeft : 3,
		background : "white",
		color : "black"
	},

	".hintLabel" : {
		top : 315,
		color : "rgba(255, 255, 255, 0.90)",
		textShadowColor : "rgba(0, 0, 0, 0.4)",
		fontSize : 9,
		radius : 3,
		paddingLeft : 4,
		paddingRight : 4,
		shadowBlur : 30
	}

}









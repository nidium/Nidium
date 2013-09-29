/* --------------------------------------------------------------------------- *
 * NSS : Native Style Sheet                                (c) 2013 Nidium.com * 
 * --------------------------------------------------------------------------- * 
 * Version:     0.5                                                            *
 * Template:    Native Default                                                 *
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
	/* Static Containers */
	
	"@default" : {
		elementColor : "#3388dd",

		darkBackgroundColor : "#262722",
		liteBackgroundColor : "#ffffff",
		textBackgroundColor : "#ffffff",

		textColor : "#333333",
		textSelectionColor : "rgba(0, 0, 128, 0.2)",

		fontSize : 12,
		fontFamily : "arial",

		textShadowColor : "rgba(0, 0, 0, 0.15)",
		textShadowOffsetX : 1,
		textShadowOffsetY : 1,
		textShadowBlur : 1,

		shadowColor : "rgba(0, 0, 0, 0.10)",
		shadowOffsetX : 0,
		shadowOffsetY : 2,
		shadowBlur : 4,

		radius : 4,

		cursor : {
			default : "arrow",
			hover : "pointer",
			drag : "drag"
		}
	},

	/* Element Definitions */

	"UIButton" : {
		label : "Button",
		background : "#2277e0",
		color : "#ffffff",
		cursor : "pointer",
		textAlign : "center",
		radius : 2,
		autosize : true,
		height : 22,
		paddingLeft : 10,
		paddingRight : 10,
		fontSize : 11,
		fontFamily : "arial",
		canReceiveFocus : true
	},

	/* -------------------------- */

	"UIToolTip" : {
		lineheight : 30,
		height : 30,
		borderWidth : 1,
		borderColor : "rgba(0, 0, 0, 0.04)"
	},

	"UIOption" : {
		background : "#ffffff",
		color : "#666666",
		height : 22
	},

	"UIOption:hover" : {
		background : "#444444",
		color : "#ffffff"
	},

	"UIOption:selected" : {
		background : "#4D90FE",
		color : "#ffffff"
	},

	"UIOption:disabled" : {
		background : "#cccccc",
		color : "#999999"
	},

	"UIOption:disabled+hover" : {
		background : "#882222",
		color : "#ffffff"
	},

	/* Class Definitions */

	".body" : {
		background : "#262722",
		backgroundImage : "private://assets/patterns/noisy.png",
		alpha : 0.25
	}
}
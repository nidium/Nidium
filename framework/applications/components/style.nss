/* --------------------------------------------------------------------------- *
 * NSS : Native Style Sheet Draft                          (c) 2013 Stight.com * 
 * --------------------------------------------------------------------------- * 
 * Version:     0.6                                                            *
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
	".main" : {
		background : "#262722"
	},

	".button" : function(){
		this.color = "#ffffff";
		this.fontSize = 10.5;
		this.top = 48;

		this.background = "#559933";

		this.animate(
			"top", this.top, this.top+190,
			800, null,
			Math.physics.elasticOut
		);
	},

	".label" : function(){
		this.paddingLeft = 8;
		this.paddingRight = 8;
		this.width = 150;
		this.height = 28;
		this.color = "#ffffff";
		this.background = "rgba(255, 255, 255, 0.25)";
		this.fontSize = 12;
		this.radius = 4;
	},

	".method" : {color : "#A6E22E"},
	".operand" : {color : "#F92672"},
	".number" : {color : "#AE81FF"},
	".reserved" : {color : "#66D9EF"},

	".doit" : {
		label : "Do It",
		left : 970,
		top : 10
	},

	".demo" : {
		label : "demo"
	},

	".dark" : {
		background : "#111111"
	},

	".green" : {
		background : "#668822"
	},

	".blue" : {
		background : function() "#4488EE"
	},

	"UIButton" : {
		background : function(self){
			setTimeout(function(){
				self.value = "red";
			}, 2000);
		}
	},

	".rose" : {
		background : "#882266"
	}
}
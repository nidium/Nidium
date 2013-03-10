/* --------------------------------------------------------------------------- *
 * Native FrameWork                                        (c) 2013 Stight.com * 
 * --------------------------------------------------------------------------- * 
 * Version:     2.0 Falcon Build                                               *
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

const	FLAG_TEXT_NODE = 		1 << 0,
		FLAG_SYSTEM_NODE = 		1 << 1,
		FLAG_FLOATING_NODE =	1 << 2;

var ಠ_ಠ = '2.0',
	
	__PATH_ROOT__ = 'falcon/',
	__PATH_KERNEL__ = __PATH_ROOT__ + 'kernel/',
	__PATH_LAYOUT__ = __PATH_ROOT__ + 'layout/',
	__PATH_PLUGINS__ = __PATH_ROOT__ + 'plugins/';

/* -------------------------------------------------------------------------- */

/* Todo : add more performance flags */
var	__ENABLE_TEXT_SHADOWS__ = true,
	__ENABLE_BUTTON_SHADOWS__ = true,
	__ENABLE_IMAGE_INTERPOLATION__ = true,
	__ENABLE_GRADIENT_LAYERS__ = true;

/* -------------------------------------------------------------------------- */

require(__PATH_KERNEL__ + 'helper.inc.js');
require(__PATH_KERNEL__ + 'engine.inc.js');
require(__PATH_KERNEL__ + 'kernel.inc.js');
require(__PATH_KERNEL__ + 'object.inc.js');
require(__PATH_KERNEL__ + 'extend.inc.js');
require(__PATH_KERNEL__ + 'events.inc.js');
require(__PATH_KERNEL__ + 'motion.inc.js');
require(__PATH_KERNEL__ + 'opengl.inc.js');
require(__PATH_KERNEL__ + 'audio.inc.js');

/* -- UI Elements -- */
require(__PATH_LAYOUT__ + 'UIElement.js');
require(__PATH_LAYOUT__ + 'UIView.js');
require(__PATH_LAYOUT__ + 'UILabel.js');

require(__PATH_LAYOUT__ + 'UIButton.js');
require(__PATH_LAYOUT__ + 'UIButtonClose.js');
require(__PATH_LAYOUT__ + 'UIButtonDown.js');

require(__PATH_LAYOUT__ + 'UISliderKnob.js');
require(__PATH_LAYOUT__ + 'UISliderController.js');

require(__PATH_LAYOUT__ + 'UITab.js');
require(__PATH_LAYOUT__ + 'UITabController.js');

require(__PATH_LAYOUT__ + 'UIWindow.js');
require(__PATH_LAYOUT__ + 'UIWindowResizer.js');
require(__PATH_LAYOUT__ + 'UIStatus.js');

require(__PATH_LAYOUT__ + 'UIOption.js');
require(__PATH_LAYOUT__ + 'UIDropDownController.js');

require(__PATH_LAYOUT__ + 'UIScrollBars.js');
require(__PATH_LAYOUT__ + 'UISpinner.js');
require(__PATH_LAYOUT__ + 'UIModal.js');
require(__PATH_LAYOUT__ + 'UILine.js');
require(__PATH_LAYOUT__ + 'UIToolTip.js');

require(__PATH_LAYOUT__ + 'UITextNode.js');
require(__PATH_LAYOUT__ + 'UITextController.js');
require(__PATH_LAYOUT__ + 'UITextMatrix.js');

//require(__PATH_LAYOUT__ + 'UIParticle.js');

require(__PATH_LAYOUT__ + 'UIDiagram.js');
require(__PATH_LAYOUT__ + 'UIDiagramController.js');


/* -- Canvas Plugins -- */
require(__PATH_PLUGINS__ + 'misc.inc.js');
require(__PATH_PLUGINS__ + 'roundbox.inc.js');
require(__PATH_PLUGINS__ + 'tabbox.inc.js');
require(__PATH_PLUGINS__ + 'spline.inc.js');

/* -- Start Layout -- */
require(__PATH_KERNEL__ + 'layout.inc.js');

/* -------------------------------------------------------------------------- */

Native.core = {
	init : function(){
		if ("console" in Native.scope) {
			console.show();
			console.clear();
		} else {
			Native.scope.console = {
				show : function(){},
				hide : function(){},
				clear : function(){},
				log : echo
			};
		}
		this.setRenderingLoop();
		this.addStatusBar();
	},

	setRenderingLoop : function(){
		Native.layout.draw();
		/*
		window.requestAnimationFrame(function(){
			Native.FPS.start();
			Native.layout.draw();
			if (Native.layout.drawHook) Native.layout.drawHook();
			Native.FPS.show();
		});
		*/
	},

	addStatusBar : function(){
		/*
		document.status = new UIStatus(document);
		document.status.open();
		document.status.progressBarColor = "rgba(210, 255, 60, 1)";
		*/
	}
};


Native.core.init();
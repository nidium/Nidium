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

var ಠ_ಠ = '2.5',
	
	__PATH_KERNEL__ = 'kernel/',
	__PATH_LAYOUT__ = 'layout/',
	__PATH_PLUGINS__ = 'plugins/';

/* -------------------------------------------------------------------------- */

/* Todo : add more performance flags */
const	__ENABLE_TEXT_SHADOWS__ = true,
		__ENABLE_BUTTON_SHADOWS__ = true,
		__ENABLE_IMAGE_INTERPOLATION__ = true,

		__MAX_INPUT_LENGTH__ = 2048,
		__MAX_LAYER_WIDTH__ = 16364;

/* -------------------------------------------------------------------------- */

load(__PATH_KERNEL__ + 'helper.inc.js');
load(__PATH_KERNEL__ + 'engine.inc.js');
load(__PATH_KERNEL__ + 'kernel.inc.js');
load(__PATH_KERNEL__ + 'object.inc.js');
load(__PATH_KERNEL__ + 'extend.inc.js');
load(__PATH_KERNEL__ + 'events.inc.js');
load(__PATH_KERNEL__ + 'motion.inc.js');
load(__PATH_KERNEL__ + 'opengl.inc.js');
load(__PATH_KERNEL__ + 'audio.inc.js');
load(__PATH_KERNEL__ + 'video.inc.js');

/* -- UI Elements -- */

load(__PATH_LAYOUT__ + 'UIElement.js');
load(__PATH_LAYOUT__ + 'UIView.js');
load(__PATH_LAYOUT__ + 'UIPath.js');
load(__PATH_LAYOUT__ + 'UILabel.js');

load(__PATH_LAYOUT__ + 'Icon.js');

load(__PATH_LAYOUT__ + 'UIButton.js');
load(__PATH_LAYOUT__ + 'UIButtonClose.js');
load(__PATH_LAYOUT__ + 'UIButtonDown.js');

load(__PATH_LAYOUT__ + 'UIRadio.js');

load(__PATH_LAYOUT__ + 'UISliderKnob.js');
load(__PATH_LAYOUT__ + 'UISliderController.js');

load(__PATH_LAYOUT__ + 'UITab.js');
load(__PATH_LAYOUT__ + 'UITabController.js');

load(__PATH_LAYOUT__ + 'UIWindow.js');
load(__PATH_LAYOUT__ + 'UIWindowResizer.js');
load(__PATH_LAYOUT__ + 'UIStatus.js');

load(__PATH_LAYOUT__ + 'UIOption.js');
load(__PATH_LAYOUT__ + 'UIDropDownController.js');

load(__PATH_LAYOUT__ + 'UIScrollBars.js');
load(__PATH_LAYOUT__ + 'UISpinner.js');
load(__PATH_LAYOUT__ + 'UIModal.js');
load(__PATH_LAYOUT__ + 'UILine.js');
load(__PATH_LAYOUT__ + 'UIToolTip.js');

load(__PATH_LAYOUT__ + 'UIVideo.js');

load(__PATH_LAYOUT__ + 'UITextInput.js');
load(__PATH_LAYOUT__ + 'UITextField.js');
load(__PATH_LAYOUT__ + 'UITextNode.js');
load(__PATH_LAYOUT__ + 'UITextController.js');
load(__PATH_LAYOUT__ + 'UITextMatrix.js');

load(__PATH_LAYOUT__ + 'UIParticle.js');

load(__PATH_LAYOUT__ + 'UIDiagram.js');
load(__PATH_LAYOUT__ + 'UIDiagramController.js');


/* -- Canvas Plugins -- */
load(__PATH_PLUGINS__ + 'misc.inc.js');
load(__PATH_PLUGINS__ + 'roundbox.inc.js');
load(__PATH_PLUGINS__ + 'tabbox.inc.js');
load(__PATH_PLUGINS__ + 'spline.inc.js');
load(__PATH_PLUGINS__ + 'path.inc.js');

/* -- Start Layout -- */
load(__PATH_KERNEL__ + 'layout.inc.js');

/* -------------------------------------------------------------------------- */

Native.core = {
	init : function(){
		this.createDocument();
		this.createContextMenu();
		this.createSpinner();
		this.setRenderingLoop();
		this.addStatusBar();
		delete(this.init);
	},

	createSpinner : function(){
		document.spinner = new UISpinner("document", {

		});

		document.spinner = new UISpinner(document, {
			height : 40,
			width : 40,
			dashes : 12,
			lineWidth : 8,
			color : "rgba(128, 128, 128, 0.5)",
			speed : 32,
			opacity : 0.3,
			radius : 2
		}).center();
	},

	onready : function(){
		document.spinner.fadeOut(250, function(){
			this.stop().hide();
		});
		delete(this.onready);
	},

	createDocument : function(){
		var doc = new Application({
			id : "document",
			left : 0,
			top : 0,
			width : window.width,
			height : window.height,
			background : "#ffffff", // "#272822"
			cursor : "arrow",
			canReceiveFocus : true,
			outlineOnFocus : false
		});

		doc.stylesheet = document.stylesheet;

		doc.style = {
			get : function(selector){
				return document.stylesheet[selector];
			},

			set : function(selector, properties){
				Native.StyleSheet.setProperties(selector, properties);
			},

			add : function(selector, properties){
				Native.StyleSheet.mergeProperties(selector, properties);
			}
		};

		Object.createProtectedElement(Native.scope, "document", doc);
	},

	createContextMenu : function(){
		document.addEventListener("contextmenu", function(e){
			var root = e.element._root;

			if (document.overlayView) document.overlayView.remove();

			document.overlayView = root.add("UIElement", {
				background : "rgba(0, 0, 0, 0.7)"
			});

			document.contextMode = false;

			document.overlayView.addEventListener("mousedown", function(e){
				if (document.contextMode === false) return;
				if (document.contextMenu) {
					document.contextMenu.selector.hide();
					document.contextMenu.remove();
					this.remove();
				}
			});

			var	myMenu = [
				/* Tab 0 */ {label : "Copy", 		value : 0},
				/* Tab 1 */ {label : "Cut", 		value : 1},
				/* Tab 2 */ {label : "Paste", 		value : 2},
				/* Tab 3 */ {label : "View Source",	value : 10}
			];

			if (this.contextMenu) {
				this.contextMenu.selector.hide();
				this.contextMenu.remove();
			}

			this.contextMenu = document.overlayView.add("UIDropDownController", {
				left : 588,
				top : 80,
				maxHeight : 198,
				hideSelector : true,
				hideToggleButton : true,
				name : "documentContextMenu",
				radius : 2,
				elements : myMenu,
				background : '#333333',
				selectedBackground : "#4D90FE",
				selectedColor : "#FFFFFF"
			});

			this.contextMenu.addEventListener("blur", function(){
				this.fireEvent("tick", {})
				this.remove();
			});

			this.contextMenu.place(
				window.mouseX,
				window.mouseY
			).unselect().openSelector(0);

			this.contextMenu.addEventListener("select", function(){
				document.contextMenu.selector.hide();
				document.contextMenu.remove();
				document.overlayView.remove();
			});

			setTimeout(function(){
				document.contextMode = true;
			}, 5);
		});
	},

	setRenderingLoop : function(){
		Native.layout.draw();
		window.requestAnimationFrame(function(){
			if (Native.layout.drawHook) Native.layout.drawHook();
		});
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
window._onready = function(){
	Native.core.onready();
};

window._onassetready = function(e){
//	console.log("asset read", e.tag, e.id);
};


/* --------------------------------------------------------------------------- *
 * Native FrameWork                                        (c) 2013 nidium.com * 
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
		__ENABLE_CONTROL_LAYERS__ = true,
		__ENABLE_CONTROL_SHADOWS__ = true,
		__ENABLE_IMAGE_INTERPOLATION__ = true,

		__MAX_INPUT_LENGTH__ = 2048,
		__MAX_LAYER_WIDTH__ = 16364;

/* -------------------------------------------------------------------------- */

load(__PATH_KERNEL__ + 'helper.inc.js');
load(__PATH_KERNEL__ + 'math.inc.js');
load(__PATH_KERNEL__ + 'engine.inc.js');
load(__PATH_KERNEL__ + 'NDMElement.constructor.js');
load(__PATH_KERNEL__ + 'NDMElement.method.js');
load(__PATH_KERNEL__ + 'NDMElement.prototype.js');
load(__PATH_KERNEL__ + 'NDMElement.updater.js');
load(__PATH_KERNEL__ + 'NDMElement.listeners.js');
load(__PATH_KERNEL__ + 'NDMElement.draw.js');
load(__PATH_KERNEL__ + 'NDMElement.proxy.js');
load(__PATH_KERNEL__ + 'NDMElement.motion.js');
load(__PATH_KERNEL__ + 'NDMElement.parser.js');
load(__PATH_KERNEL__ + 'extend.inc.js');
load(__PATH_KERNEL__ + 'events.inc.js');
load(__PATH_KERNEL__ + 'opengl.inc.js');
load(__PATH_KERNEL__ + 'audio.inc.js');
load(__PATH_KERNEL__ + 'video.inc.js');

/* -- Layout Parser and Renderer -- */
load(__PATH_KERNEL__ + 'document.layout.js');
load(__PATH_KERNEL__ + 'document.selectors.js');
load(__PATH_KERNEL__ + 'document.nss.js');


/* -- UI Elements -- */

load(__PATH_LAYOUT__ + 'UIElement.js');
load(__PATH_LAYOUT__ + 'UIView.js');
load(__PATH_LAYOUT__ + 'UIListView.js');
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

/* -------------------------------------------------------------------------- */

Object.createProtectedElement(window.scope, "Application", function(options){
	options = options || {};
	options.canReceiveFocus = true;
	options.outlineOnFocus = false;

	var element = new NDMElement("UIView", options, null);
	element._root = element;

	Native.elements.init(element);

	document.layout.register(element);
	document.layout.update();

	return element;
});

/* -------------------------------------------------------------------------- */

Native.core = {
	init : function(){
		this.createDocument();
		this.extendDocument();
		this.drawLayout();
		delete(this.init);
	},

	onready : function(){
		document.spinnerElement.fadeOut(250, function(){
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
			background : "#000000", // "#272822"
			cursor : "arrow",
			canReceiveFocus : true,
			outlineOnFocus : false
		});

		for (var p in document) {
			doc[p] = document[p];
		}

		Object.createProtectedElement(window.scope, "document", doc);
	},

	extendDocument : function(){
		document.addEventListener("contextmenu", function(e){
			var root = e.element._root;

			if (document.overlayView) {
				document.overlayView.remove();
				document.overlayView = null;
				return false;
			}

			document.overlayView = root.add("UIElement", {
				background : "rgba(255, 255, 255, 0.1)"
			});

			document.contextMode = false;

			document.overlayView.addEventListener("mousedown", function(e){
				if (document.contextMode === false) return;
				if (document.contextMenu) {
					document.contextMenu.selector.hide();
					document.contextMenu.remove();
					document.overlayView.remove();
					document.overlayView = null;
				}
			});

			var disabled = !(e.element.isTextZone);

			var	myMenu = [
				{label : "Copy", 		value : 0, disabled:disabled},
				{label : "Cut", 		value : 1, disabled:disabled},
				{label : "Paste", 		value : 2, disabled:disabled},
				{label : "View Source",	value : 10}
			];

			if (this.contextMenu) {
				this.contextMenu.selector.hide();
				this.contextMenu.remove();
			}

			this.contextMenu = document.overlayView.add("UIDropDownController", {
				left : 588,
				top : 80,
				hideSelector : true,
				hideToggleButton : true,
				name : "documentContextMenu",
				radius : 2,
				background : '#333333'
			});

			this.contextMenu.setOptions(myMenu);

			this.contextMenu.addEventListener("blur", function(){
				this.fireEvent("tick", {});
				this.remove();
			});

			this.contextMenu.place(
				window.mouseX,
				window.mouseY
			).unselect().openSelector(0);

			this.contextMenu.addEventListener("select", function(e){
				document.contextMenu.selector.hide();
				document.contextMenu.remove();
				document.overlayView.remove();
				document.overlayView = null;
				console.log(e.value);
			});

			setTimeout(function(){
				document.contextMode = true;
			}, 5);
		});

		document.spinnerElement = new UISpinner(document, {
			height : 40,
			width : 40,
			dashes : 12,
			lineWidth : 8,
			color : "rgba(128, 128, 128, 0.5)",
			speed : 32,
			opacity : 0.3,
			radius : 2
		}).center();

		document.status = new UIStatus(document);
		document.status.progressBarColor = "rgba(210, 255, 60, 1)";
		document.status.progressBarLeft = 70;
	},

	drawLayout : function(){
		document.layout.draw();
		window.events.tick();
	}
};

Native.core.init();
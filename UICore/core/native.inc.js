/* -------------------------- */
/* Native (@) 2012 Stight.com */
/* -------------------------- */

load('core/helper.inc.js');
load('core/events.inc.js');

load('core/layout/core.js');

load('core/layout/elements/UIView.js');
load('core/layout/elements/UIButton.js');
load('core/layout/elements/UIButtonClose.js');
load('core/layout/elements/UIButtonDown.js');
load('core/layout/elements/UIText.js');
load('core/layout/elements/UIScrollBars.js');
load('core/layout/elements/UIRadio.js');
load('core/layout/elements/UILines.js');

load('core/layout/elements/UITab.js');
load('core/layout/elements/UITabController.js');

load('core/layout/elements/UIDropDownOption.js');
load('core/layout/elements/UIDropDownController.js');

load('core/layout/elements/UIWindow.js');


load('core/plugins/blur.inc.js');
load('core/plugins/roundbox.inc.js');
load('core/plugins/tabbox.inc.js');
load('core/plugins/animations.inc.js');

canvas.__mustBeDrawn = true;

var layout = {
	objID : 0,

	nodes : {},
	elements : [],

	higherzIndex : 0,

	register : function(element){
		this.nodes['_obj_' + this.objID++] = element;
	},

	registerClone : function(element){
		this.nodes[element.id] = element;
	},

	remove : function(element){
		delete(this.nodes[element.id]);
		this.refresh();
	},

	clear : function(){
		canvas.clearRect(0, 0, window.width, window.height);
	},

	draw : function(){

		//this.clear();

		let z = this.getElements();

		if (canvas.__mustBeDrawn) {
			for (let i in z){
				if (z[i].visible){
					z[i].beforeDraw();
					z[i].draw();
					z[i].afterDraw();
				}
			}
			canvas.__mustBeDrawn = false;
		}

	},

	grid : function(){
		for (var x=0; x<canvas.width ; x+=80) {
			canvas.beginPath();
			canvas.moveTo(x, 0);
			canvas.lineTo(x, canvas.height);
			canvas.moveTo(0, x);
			canvas.lineTo(canvas.width, x);
			canvas.stroke();

		}
	},

	getElements : function(){
		let elements = [];

		var dx = function(nodes, parent){
			for (var child in nodes){
				elements.push(nodes[child]);
				if (count(nodes[child].nodes)>0) {
					dx(nodes[child].nodes, nodes[child].parent);
				}
			}
		};

		dx(this.nodes, null);

		this.elements = elements.sort(function(a, b){
			return a.zIndex - b.zIndex;
		});

		this.higherzIndex = elements[elements.length-1] ? elements[elements.length-1].zIndex : 0;
		return elements;
	},

	find : function(property, value){

		let elements = [];

		let dx = function(nodes, parent){
			for (let child in nodes){
				if (nodes[child][property] && nodes[child][property] == value){
					elements.push(nodes[child]);
				}
				if (count(nodes[child].nodes)>0) {
					dx(nodes[child].nodes, nodes[child].parent);
				}
			}
		};

		dx(this.nodes, null);

		elements.each = function(cb){
			for (var i in elements) {
				if (elements.hasOwnProperty(i) && elements[i].id){
					cb.call(elements[i]);
				}
			}
		};

		return elements;
	},

	getHigherZindex : function(){
		let zindexes = [];

		let dx = function(nodes, parent){
			for (var child in nodes){
				zindexes.push(nodes[child].zIndex);
				if (count(nodes[child].nodes)>0) {
					dx(nodes[child].nodes, nodes[child].parent);
				}
			}
		};
		dx(this.nodes, null);
		return zindexes.length ? Math.max.apply(null, zindexes) : 0;
	},

	refresh : function(){
		//this.draw();
	}

};

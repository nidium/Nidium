/* ------------------------+------------- */
/* Native Framework 2.0    | Falcon Build */
/* ------------------------+------------- */
/* (c) 2013 Stight.com - Vincent Fontaine */
/* -------------------------------------- */

"use strict";

/* -------------------------------------------------------------------------- */

Native.system = {
	doubleClickInterval : 250
};

/* -------------------------------------------------------------------------- */

Native.events = {
	options : {
		pointerHoldTime : 600
	},

	last : null,

	mousedown : false,
	doubleclick : false,
	preventmouseclick : false,
	preventdefault : false,

	dragging : false,
	dragstarted : false,

	cloneElement : null,	
	sourceElement : null,

	__dataTransfer : {
		data : {},

		set : function(o){
			for (var i in o){
				if (o.hasOwnProperty(i)){
					this.data[i] = o[i];
				}
			}
		},

		setData : function(key, value){
			this.data[key] = value;
		},

		getData : function(key){
			return this.data[key];
		},

		effectAllowed : function(){

		},

		reset : function(){
			delete(this.data);
			this.data = {};
		}
	},

	stopDrag : function(){
		this.mousedown = false;
		this.dragging = false;
		this.dragstarted = false;
		this.sourceElement = null;
	},

	setSource : function(e, source, target){
		e.source = source;
		e.target = target;
	},

	hook : function(element, e){
		if (typeof window.onElementUnderPointer == "function"){
			window.onElementUnderPointer.call(element, e);
		}
	},

	tick : function(){
		this.dispatch("tick", {
			x : window.mouseX,
			y : window.mouseY,
			xrel : 0,
			yrel : 0
		});
	},

	dispatch : function(name, e){
		var self = this,
			x = e.x,
			y = e.y,

			cancelBubble = false,
			cancelEvent = false,

			z = document.getElements();

		e.preventDefault = function(){
			self.preventdefault = true;
		};

		e.preventMouseEvents = function(){
			self.preventmouseclick = true;
		};

		e.cancelBubble = function(){
			cancelBubble = true;
		};

		e.stopPropagation = function(){
			cancelBubble = true;
			cancelEvent = true;
		};

		e.forcePropagation = function(){
			cancelBubble = false;
			cancelEvent = false;
		};

		var __mostTopElementHooked = false;

		if (name == "mousedown" || name == "mouseup") {
			if (this.preventmouseclick) {
				this.preventmouseclick = false;
				return;
			}
		}

		Native.layout.topElement = false;

		for(var i=z.length-1 ; i>=0 ; i--) {
			var element = z[i];
			cancelEvent = false;

			if (!element.layer.__visible) {
				continue;
			}

			if (this.preventdefault && element == document) {
				this.preventdefault = false;
				continue;
			}

			if (name=='keydown'){
				if (e.keyCode == 1073742051 || e.keyCode == 1073742055) {
					element.cmdKeyDown = true;
				}
				if (e.keyCode == 1073742049 || e.keyCode == 1073742053) {
					element.shiftKeyDown = true;
				}
			}

			if (name=='keyup'){
				if (e.keyCode == 1073742051 || e.keyCode == 1073742055) {
					element.cmdKeyDown = false;
				}
				if (e.keyCode == 1073742049 || e.keyCode == 1073742053) {
					element.shiftKeyDown = false;
				}
			}

			e.shiftKeyDown = element.shiftKeyDown == undefined ?
							false : element.shiftKeyDown;

			e.cmdKeyDown = element.cmdKeyDown == undefined ?
							false : element.cmdKeyDown;

			if (name=='keydown' || name=='keyup' || name=='textinput'){
				if (element.canReceiveKeyboardEvents){
					element.fireEvent(name, e);
				}
			} else if (element.isPointInside(x, y)){

				if (Native.layout.topElement === false) {
					Native.layout.topElement = element;
				}

				if (__mostTopElementHooked === false){
					if (element._background || element._backgroundImage){
						Native.events.hook(element, e);
						this.mostTopElementUnderMouse = element;
						__mostTopElementHooked = true;
					}
				}

				switch (name) {
					case "contextmenu" :
						e.element = Native.layout.topElement;
						break;

					case "mousewheel" :
						//cancelBubble = true;
						break;

					case "tick" :
						this.setSource(e, this.sourceElement, element);
						cancelBubble = this.fireMouseOver(element, e);
						break;

					case "mousemove" :
						this.setSource(e, this.sourceElement, element);
						cancelBubble = this.fireMouseOver(element, e);
						break;

					case "drag" :
						cancelEvent = true;
						if (!e.source) {
							this.stopDrag();
						}
						break;

					case "dragover" :
						this.setSource(e, this.sourceElement, element);

						if (!e.source) {
							cancelEvent = true;
							this.stopDrag();
						}

						cancelBubble = this.fireMouseOver(element, e);
						break;

					case "dragstart" :
						e.source = element;
						e.target = element;
						e.source.dragendFired = false;

						this.sourceElement = element;

						if (element.draggable) {
							this.cloneElement = element.clone();
						}
						cancelBubble = true;
						break;

					case "drop":
						this.setSource(e, this.sourceElement, element);

						if (!e.source.dragendFired){
							e.source.dragendFired = true;
							e.source.fireEvent("dragend", e);
						}
						break;

					default : break;
				}
				
				if (cancelEvent===false){
					element.fireEvent(name, e);
				}

			} else {
				e.source = this.sourceElement || element;
				e.target = element;
				cancelBubble = this.fireMouseOut(element, e);
			}

			if (cancelBubble) break;
		}

		if (name=="drag"){
			e.source = this.sourceElement;
			if (e.source) {
				e.source.fireEvent("drag", e);
			}
		}
	},

	/* -- VIRTUAL EVENTS PROCESSING ----------------------------------------- */

	fireMouseOver : function(element, e){
		if (!element.mouseover) {
			element.mouseover = true;
			element.mouseout = false;
			if (this.dragging) {
				element.fireEvent("dragenter", e);
			} else {
				element.fireEvent("mouseover", e);
			}
			return false;
		}
		
		if (this.mostTopElementUnderMouse) {
			window.cursor = this.mostTopElementUnderMouse.disabled ?
					"arrow" : this.mostTopElementUnderMouse.cursor;
		}
	},

	fireMouseOut : function(element, e){
		if (element.mouseover && !element.mouseout) {
			element.mouseover = false;
			element.mouseout = true;
			if (this.dragging) {
				element.fireEvent("mouseout", e);
				element.fireEvent("mouseleave", e);
				element.fireEvent("dragleave", e);
				return false;
			} else {
				element.fireEvent("mouseout", e);
				element.fireEvent("mouseleave", e);
				return true;
			}
		}
	},

	/* -- PHYSICAL EVENTS PROCESSING ---------------------------------------- */

	mousedownEvent : function(e){
		window.mouseX = e.x;
		window.mouseY = e.y;
		var o = this.last || {x:0, y:0},
			dist = Math.distance(o.x, o.y, e.x, e.y) || 0;

		if (e.which == 3) {
			this.preventmouseclick = false;
			this.dispatch("contextmenu", e);
		}
		this.mousedown = true;
		this.dragstarted = false;

		e.time = +new Date();
		e.duration = null;
		e.dataTransfer = this.__dataTransfer;

		this.dispatch("mousedown", e);

		if (this.last){
			e.duration = e.time - this.last.time;

			if (dist<3 && e.duration <= Native.system.doubleClickInterval) {
				this.dispatch("mousedblclick", e);
				this.doubleclick = true;
			}
		}

		this.last = e;
	},

	mousemoveEvent : function(e){
		window.mouseX = e.x;
		window.mouseY = e.y;
		e.dataTransfer = this.__dataTransfer;

		if (this.mousedown){

			if (this.dragstarted){
				this.dragging = true;
				this.dispatch("dragover", e);
				this.dispatch("drag", e);

				let (ghost = this.cloneElement){
					if (ghost) {
						ghost.left += e.xrel;
						ghost.top += e.yrel;
					}
				}

			} else {
				this.dragstarted = true;
				this.dispatch("dragstart", e);
			}

		} else {
			this.dispatch("mousemove", e);
		}
	},

	mouseupEvent : function(e){
		var self = this,
			o = this.last || {x:0, y:0},
			dist = Math.distance(o.x, o.y, e.x, e.y) || 0,
			elapsed = 0;

		// prevent Mouse UP to be fired after (MouseDown + CTRL R)
		if (this.last) {
			elapsed = (+new Date()) - this.last.time;
		}

		this.mousedown = false;

		e.dataTransfer = this.__dataTransfer;

		if (this.dragstarted && this.sourceElement){
			if (this.cloneElement){
				this.cloneElement.remove();
				delete(this.cloneElement);
			}

			this.dispatch("drop", e);
		}
		this.dragging = false;
		this.dragstarted = false;
		this.sourceElement = null;

		this.dispatch("mouseup", e);

		if (o && dist<3) {
			if (elapsed > this.options.pointerHoldTime) {
				this.doubleclick = false;
				this.timer = false;
				this.dispatch("mouseholdup", e);
				this.dispatch("mouseclick", e);
			} else {
				this.dispatch("mouseclick", e);
			}
		}
	},

	mousewheelEvent : function(e){
		this.dispatch("mousewheel", e);
	},

	keydownEvent : function(e){
		this.dispatch("keydown", e);

		if (e.keyCode == 9) {
			Native.layout.focusNextElement();
		}
		window.keydown = e.keyCode;
	},

	keyupEvent : function(e){
		this.dispatch("keyup", e);
		window.keydown = null;
	},

	textinputEvent : function(e){
		e.text = e.val;
		this.dispatch("textinput", e);
	}

};

/* -- DOM EVENTS IMPLEMENTATION --------------------------------------------- */

DOMElement.implement({

	mouseover : false,
	mouseout : false,
	dragendFired : false,

	fireEvent : function(name, e, successCallback){
		var acceptedEvent = true,
			listenerResponse = true,
			cb = OptionalCallback(successCallback, null);

		if (typeof this["on"+name] == 'function'){
			if (e !== undefined){
				e.dx = e.xrel;
				e.dy = e.yrel;
				e.refuse = function(){
					acceptedEvent = false;
				};
				listenerResponse = this["on"+name](e);
				if (cb && acceptedEvent) cb.call(this);

				return OptionalBoolean(listenerResponse, true);
			} else {
				listenerResponse = this["on"+name]();
			}
		} else {
			if (cb) cb.call(this);
		}
		return this;
	},

	addEventListener : function(name, callback, propagation){
		var self = this;
		self._eventQueues = self._eventQueues ? self._eventQueues : [];
		var queue = self._eventQueues[name];

		queue = !queue ? self._eventQueues[name] = [] : 
						 self._eventQueues[name];

		queue.push({
			name : OptionalString(name, "error"),
			fn : OptionalCallback(callback, function(){}),
			propagation : OptionalBoolean(propagation, true),
			response : null
		});

		self["on"+name] = function(e){
			for(var i=queue.length-1; i>=0; i--){
				queue[i].response = queue[i].fn.call(self, e);
				if (!queue[i].propagation){
					continue;
				}
			}
			// uncomment to support return in listener (you should not)
			//if (queue && queue[queue.length-1]) return queue[queue.length-1].response;
		};
		return this;
	},

	click : function(cb){
		if (typeof cb == "function") {
			this.addEventListener("mouseclick", cb, false);
		} else {
			this.fireEvent("mouseclick", {
				x : window.mouseX,
				y : window.mouseY
			});
		}
		return this;
	},

	hoverize : function(cbOver, cbOut){
		if (typeof cbOver == "function") {
			this.addEventListener("mouseover", cbOver, false);
		}
		if (typeof cbOut == "function") {
			this.addEventListener("mouseout", cbOut, false);
		}
		return this;
	},

	drag : function(cb){
		if (typeof cb == "function") {
			this.addEventListener("drag", cb, false);
		}
		return this;
	}
});

/* -- THREAD EVENT LISTENER ------------------------------------------------- */

Thread.prototype.addEventListener = function(name, callback, propagation){
	var self = this;
	self._eventQueues = self._eventQueues ? self._eventQueues : [];
	var queue = self._eventQueues[name];

	queue = !queue ? self._eventQueues[name] = [] : 
					 self._eventQueues[name];

	queue.push({
		name : OptionalString(name, "error"),
		fn : OptionalCallback(callback, function(){}),
		propagation : OptionalBoolean(propagation, true),
		response : null
	});

	self["on"+name] = function(e){
		for(var i=queue.length; i>=0; i--){
			queue[i].response = queue[i].fn.call(self, e);
			if (!queue[i].propagation){
				continue;
			}
		}
	};
	return this;
};

Object.attachEventListener = function(obj){
	obj.addEventListener = function(name, callback, propagation){
		var self = this;
		self._eventQueues = self._eventQueues ? self._eventQueues : [];
		var queue = self._eventQueues[name];

		queue = !queue ? self._eventQueues[name] = [] : 
						 self._eventQueues[name];

		queue.push({
			name : OptionalString(name, "error"),
			fn : OptionalCallback(callback, function(){}),
			propagation : OptionalBoolean(propagation, true),
			response : null
		});

		self["on"+name] = function(e){
			for(var i=queue.length-1; i>=0; i--){
				queue[i].response = queue[i].fn.call(self, e);
				if (!queue[i].propagation){
					continue;
				}
			}
		};
		return this;
	}
};

/* -- MOUSE EVENTS ---------------------------------------------------------- */

window._onmousedown = function(e){
	Native.events.mousedownEvent(e);
};

window._onmousemove = function(e){
	Native.events.mousemoveEvent(e);
};

window._onmousewheel = function(e){
	Native.events.mousewheelEvent(e);
};

window._onmouseup = function(e){
	Native.events.mouseupEvent(e);
};

/* -- KEYBOARD EVENTS ------------------------------------------------------- */

window._onkeydown = function(e){
	Native.events.keydownEvent(e);
};

window._onkeyup = function(e){
	Native.events.keyupEvent(e);
};

window._ontextinput = function(e){
	Native.events.textinputEvent(e);
};

/* -- WINDOW EVENTS --------------------------------------------------------- */

window._onfocus = function(e){
	
};

window._onblur = function(e){
	
};

/* -- LOAD EVENTS ----------------------------------------------------------- */

window._onready = function(){
	Native.core.onready();
};

window._onassetready = function(e){
	switch (e.tag) {
		case "style" :
			Native.StyleSheet.refresh();
			break;
		default : break
	}
};


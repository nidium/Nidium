/* -------------------------- */
/* Native (@) 2012 Stight.com */
/* -------------------------- */

canvas.onmousedown = function(e){
	UIEvents.__mouseDown = true;
	UIEvents.__startDragLaunched = false;

	e.time = +new Date();
	e.duration = null;
	e.dataTransfer = UIEvents.__dataTransfer;

	if (UIEvents.__lastClickEvent){
		e.duration = e.time - UIEvents.__lastClickEvent.time;
	}
	UIEvents.__lastClickEvent = e;

	canvas.__mustBeDrawn = true;
	UIEvents.dispatch("mousedown", e);

};

canvas.onmousemove = function(e){
	canvas.mouseX = e.x;
	canvas.mouseY = e.y;
	e.dataTransfer = UIEvents.__dataTransfer;

	if (UIEvents.__mouseDown){

		canvas.__mustBeDrawn = true;
		
		if (UIEvents.__startDragLaunched){
			UIEvents.__dragging = true;
			UIEvents.dispatch("dragover", e);
			UIEvents.dispatch("drag", e);

			let (ghost = UIEvents.__dragGhost){
				if (ghost) {
					ghost.left = e.xrel + (ghost.x);
					ghost.top = e.yrel + (ghost.y);
				}
			}

		} else {
			UIEvents.__startDragLaunched = true;
			UIEvents.dispatch("dragstart", e);
		}

	} else {
		UIEvents.dispatch("mousemove", e);
	}
};

canvas.onmousewheel = function(e){
	UIEvents.dispatch("mousewheel", e);
};


canvas.onmouseup = function(e){
	var o = UIEvents.__lastClickEvent || {x:0, y:0},
		d = Math.distance(o.x, o.y, e.x, e.y) || 0,
		timeEllapsedFromMouseDown = 0;

	canvas.__mustBeDrawn = true;

	// prevent MouseDown + CTRL R + Mouse UP error
	if (UIEvents.__lastClickEvent) {
		timeEllapsedFromMouseDown = (+new Date()) - UIEvents.__lastClickEvent.time;
	}

	UIEvents.__mouseDown = false;

	e.dataTransfer = UIEvents.__dataTransfer;

	if (UIEvents.__startDragLaunched && UIEvents.__dragSourceElement){
		if (UIEvents.__dragGhost){
			UIEvents.__dragGhost.remove();
			delete(UIEvents.__dragGhost);
		}

		UIEvents.dispatch("drop", e);
	}
	UIEvents.__dragging = false;
	UIEvents.__startDragLaunched = false;
	UIEvents.__dragSourceElement = null;

	//if (!UIEvents.__timerEngaged) {
		UIEvents.dispatch("mouseup", e);
	//}

	if (o && d<5) {

		if (timeEllapsedFromMouseDown > UIEvents.devices.pointerHoldTime) {
			UIEvents.__doubleClickLaunched = false;
			UIEvents.__timerEngaged = false;
			//UIEvents.dispatch("mouselongclick", e);
			UIEvents.dispatch("mouseclick", e);
		} else {

			if (!UIEvents.__timerEngaged) {
				setTimer(function(){
					if (!UIEvents.__doubleClickLaunched) {
						UIEvents.dispatch("mouseclick", e);
					}
					UIEvents.__timerEngaged = false;
					UIEvents.__doubleClickLaunched = false;
				}, 10);

				UIEvents.__timerEngaged = true;
			} else {
				UIEvents.dispatch("mousedblclick", e);
				UIEvents.__doubleClickLaunched = true;
				UIEvents.__timerEngaged = false;
			}
		}

	}

};

/*
	EVENT_PROP("keyCode", INT_TO_JSVAL(keycode));
	EVENT_PROP("altKey", BOOLEAN_TO_JSVAL(mod & NATIVE_KEY_ALT));
	EVENT_PROP("ctrlKey", BOOLEAN_TO_JSVAL(mod & NATIVE_KEY_CTRL));
	EVENT_PROP("shiftKey", BOOLEAN_TO_JSVAL(mod & NATIVE_KEY_SHIFT));
	EVENT_PROP("repeat", BOOLEAN_TO_JSVAL(repeat));
*/

canvas.onkeydown = function(e){
	UIEvents.dispatch("keydown", e);

	if (e.keyCode == 9) {
		NativeRenderer.focusNextElement();
	}
};

canvas.onkeyup = function(e){
	UIEvents.dispatch("keyup", e);
};

canvas.ontextinput = function(e){
	e.text = e.val;
	UIEvents.dispatch("textinput", e);
};

var UIEvents = {
	devices : {
		doubleTapTime : 450,
		pointerHoldTime : 600
	},

	__mouseDown : false,
	__lastClickEvent : null,

	__timerEngaged : false,
	__doubleClickLaunched : false,

	__dragging : false,
	__dragGhost : null,	
	__dragSourceElement : null,
	__startDragLaunched : false,

	__dataTransfer : {
		data : {},

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
		this.__mouseDown = false;
		this.__dragging = false;
		this.__startDragLaunched = false;
		this.__dragSourceElement = null;
	},

	dispatch : function(eventName, e){
		var x = e.x,
			y = e.y,
			elements = [];

		var z = NativeRenderer.getElements();

		var cancelBubble = false;

		e.stopPropagation = function(){
			cancelBubble = true;
			cancelEvent = true;
		};

		for(var i=z.length-1 ; i>=0 ; i--) {
			var view = z[i],
				cancelEvent = false;

			if (!view.visible) {
				continue;
			}

			if (eventName=='keydown' || eventName=='keyup' || eventName=='textinput'){
				if (cancelEvent===false){
					view.fireEvent(eventName, e);
				}
			}

			if (view.isPointInside(x, y)){

				switch (eventName) {
					case "mousewheel" :
						cancelBubble = true;
						break;

					case "mousemove" :
						e.source = this.__dragSourceElement;
						e.target = view;
						cancelBubble = view.fireMouseOver(e);
						break;

					case "drag" :
						cancelEvent = true;
						if (!e.source) {
							this.stopDrag();
						}
						break;

					case "dragover" :
						e.source = this.__dragSourceElement;
						e.target = view;

						if (!e.source) {
							cancelEvent = true;
							this.stopDrag();
						}

						cancelBubble = view.fireMouseOver(e);
						break;

					case "dragstart" :
						e.source = view;
						e.target = view;
						e.source.flags._dragendCallend = false;

						this.__dragSourceElement = view;

						if (view.draggable) {
							this.__dragGhost = view.clone();
						}
						cancelBubble = true;
						break;

					case "drop":

						e.source = this.__dragSourceElement;
						e.target = view;

						//if (e.source.id != e.target.id && e.source.flags._dragendCallend===false){
						if (!e.source.flags._dragendCallend){
							e.source.flags._dragendCallend = true;
							e.source.fireEvent("dragend", e);
						}
						//}

				}
				
				if (cancelEvent===false){
					view.fireEvent(eventName, e);
				}


			} else {
				e.source = this.__dragSourceElement || view;
				e.target = view;
				cancelBubble = view.fireMouseOut(e);
			}

			if (cancelBubble) break;

		}

		if (eventName=="drag"){
			e.source = this.__dragSourceElement;
			if (e.source) {
				e.source.fireEvent("drag", e);
			}
		}


	}

};

UIView.implement({
	fireEvent : function(eventName, e){
		canvas.__mustBeDrawn = true;
		if (typeof this["on"+eventName] == 'function'){
			e.dx = e.xrel / this._scale;
			e.dy = e.yrel / this._scale;
			this["on"+eventName](e);
		}
	},

	fireMouseOver : function(e){
		if (!this.flags._mouseoverCalled) {
			this.flags._mouseoverCalled = true;
			this.flags._mouseoutCalled = false;
			
			if (UIEvents.__dragging) {
				if (this.ondragenter && typeof(this.ondragenter)=="function"){
					this.ondragenter(e);
					canvas.__mustBeDrawn = true;
					return false;
				}
			} else {
				if (this.onmouseover && typeof(this.onmouseover)=="function"){
					this.onmouseover(e);
					canvas.__mustBeDrawn = true;
					return false;
				}
			}
		}
	},

	fireMouseOut : function(e){
		if (this.flags._mouseoverCalled && !this.flags._mouseoutCalled) {
			this.flags._mouseoverCalled = false;
			this.flags._mouseoutCalled = true;
			if (UIEvents.__dragging) {

				/* to check */
				if (this.onmouseout && typeof(this.onmouseout)=="function"){
					this.onmouseout(e);
					canvas.__mustBeDrawn = true;
				}
				if (this.onmouseleave && typeof(this.onmouseleave)=="function"){
					this.onmouseleave(e);
					canvas.__mustBeDrawn = true;
				}
				/* -------- */

				if (this.ondragleave && typeof(this.ondragleave)=="function"){
					this.ondragleave(e);
					canvas.__mustBeDrawn = true;
					return false;
				}
			} else {
				if (this.onmouseout && typeof(this.onmouseout)=="function"){
					this.onmouseout(e);
					canvas.__mustBeDrawn = true;
				}
				if (this.onmouseleave && typeof(this.onmouseleave)=="function"){
					this.onmouseleave(e);
					canvas.__mustBeDrawn = true;
				}
				return true;
			}
		}
	},

	addEventListener : function(eventName, callback, propagation){
		var self = this,
			event = self._eventQueues[eventName];

		propagation = (propagation == "undefined") ? true : (propagation===true) ? true : false;

		event = !event ? self._eventQueues[eventName] = [] : self._eventQueues[eventName];

		event.push({
			name : eventName,
			fn : typeof(callback)=="function" ? callback : function(){},
			propagation : true
		});

		self["on"+eventName] = function(e){
			for(var i in event){
				event[i].fn.call(self, e);
				if (!event[i].propagation){
					break;
				}
			}
		};

	}
});



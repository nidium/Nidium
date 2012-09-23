/* -------------------------- */
/* Native (@) 2012 Stight.com */
/* -------------------------- */

canvas.global = {
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
	}

};

canvas.onmousedown = function(e){
	canvas.global.__mouseDown = true;
	canvas.global.__startDragLaunched = false;

	e.time = +new Date();
	e.duration = null;
	e.dataTransfer = canvas.global.__dataTransfer;

	if (canvas.global.__lastClickEvent){
		e.duration = e.time - canvas.global.__lastClickEvent.time;
	}
	canvas.global.__lastClickEvent = e;

	canvas.__mustBeDrawn = true;
	UIEvents.dispatch("mousedown", e);

};

canvas.onmousemove = function(e){
	canvas.mouseX = e.x;
	canvas.mouseY = e.y;
	e.dataTransfer = canvas.global.__dataTransfer;

	if (canvas.global.__mouseDown){

		canvas.__mustBeDrawn = true;
		
		if (canvas.global.__startDragLaunched){
			canvas.global.__dragging = true;
			UIEvents.dispatch("dragover", e);
			UIEvents.dispatch("drag", e);

			let (ghost = canvas.global.__dragGhost){
				if (ghost) {
					ghost.left = e.xrel + (ghost.x);
					ghost.top = e.yrel + (ghost.y);
				}
			}

		} else {
			canvas.global.__startDragLaunched = true;
			UIEvents.dispatch("dragstart", e);
		}

	} else {
		UIEvents.dispatch("mousemove", e);
	}
};

canvas.onmousewheel = function(e){
	e.x = canvas.mouseX;
	e.y = canvas.mouseY;
	UIEvents.dispatch("mousewheel", e);
};


canvas.onmouseup = function(e){
	var o = canvas.global.__lastClickEvent || {x:0, y:0},
		d = Math.distance(o.x, o.y, e.x, e.y) || 0,
		timeEllapsedFromMouseDown = 0;

	canvas.__mustBeDrawn = true;

	// prevent MouseDown + CTRL R + Mouse UP error
	if (canvas.global.__lastClickEvent) {
		timeEllapsedFromMouseDown = (+new Date()) - canvas.global.__lastClickEvent.time;
	}

	canvas.global.__mouseDown = false;

	e.dataTransfer = canvas.global.__dataTransfer;

	if (canvas.global.__startDragLaunched && canvas.global.__dragSourceElement){
		if (canvas.global.__dragGhost){
			canvas.global.__dragGhost.remove();
			delete(canvas.global.__dragGhost);
		}

		UIEvents.dispatch("drop", e);
	}
	canvas.global.__dragging = false;
	canvas.global.__startDragLaunched = false;
	canvas.global.__dragSourceElement = null;

	//if (!canvas.global.__timerEngaged) {
		UIEvents.dispatch("mouseup", e);
	//}

	if (o && d<5) {

		if (timeEllapsedFromMouseDown > canvas.global.devices.pointerHoldTime) {
			canvas.global.__doubleClickLaunched = false;
			canvas.global.__timerEngaged = false;
			//UIEvents.dispatch("mouselongclick", e);
			UIEvents.dispatch("mouseclick", e);
		} else {

			if (!canvas.global.__timerEngaged) {
				setTimer(function(){
					if (!canvas.global.__doubleClickLaunched) {
						UIEvents.dispatch("mouseclick", e);
					}
					canvas.global.__timerEngaged = false;
					canvas.global.__doubleClickLaunched = false;
				}, 10);

				canvas.global.__timerEngaged = true;
			} else {
				UIEvents.dispatch("mousedblclick", e);
				canvas.global.__doubleClickLaunched = true;
				canvas.global.__timerEngaged = false;
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
		layout.focusNextElement();
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
	stopDrag : function(){
		canvas.global.__mouseDown = false;
		canvas.global.__dragging = false;
		canvas.global.__startDragLaunched = false;
		canvas.global.__dragSourceElement = null;
	},

	dispatch : function(eventName, e){
		var x = e.x,
			y = e.y,
			elements = [];

		var z = layout.getElements();

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

			if (eventName=='keydown' || eventName=='keyup' ||eventName=='textinput'){
				if (cancelEvent===false){
					view.fireEvent(eventName, e);
				}
			}

			if (view.isPointInside(x, y)){

				switch (eventName) {
					case "mousemove" :
						e.source = canvas.global.__dragSourceElement;
						e.target = view;
						cancelBubble = view.throwMouseOver(e);
						break;

					case "drag" :
						cancelEvent = true;
						if (!e.source) {
							this.stopDrag();
						}
						break;

					case "dragover" :
						e.source = canvas.global.__dragSourceElement;
						e.target = view;

						if (!e.source) {
							cancelEvent = true;
							this.stopDrag();
						}

						cancelBubble = view.throwMouseOver(e);
						break;

					case "dragstart" :
						e.source = view;
						e.target = view;
						e.source.flags._dragendCallend = false;

						canvas.global.__dragSourceElement = view;

						if (view.draggable) {
							canvas.global.__dragGhost = view.clone();
						}
						cancelBubble = true;
						break;

					case "drop":

						e.source = canvas.global.__dragSourceElement;
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
				e.source = canvas.global.__dragSourceElement || view;
				e.target = view;
				cancelBubble = view.throwMouseOut(e);
			}

			if (cancelBubble) break;

		}

		if (eventName=="drag"){
			e.source = canvas.global.__dragSourceElement;
			e.source && e.source.fireEvent("drag", e);
		}


	}

};

UIView.implement({
	fireEvent : function(eventName, e){
		canvas.__mustBeDrawn = true;
		if (typeof this["on"+eventName] == 'function'){
			this["on"+eventName](e);
		}
	},

	throwMouseOver : function(e){
		if (!this.flags._mouseoverCalled) {
			this.flags._mouseoverCalled = true;
			this.flags._mouseoutCalled = false;
			
			if (canvas.global.__dragging) {
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

	throwMouseOut : function(e){
		if (this.flags._mouseoverCalled && !this.flags._mouseoutCalled) {
			this.flags._mouseoverCalled = false;
			this.flags._mouseoutCalled = true;
			if (canvas.global.__dragging) {

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
		var self = this;

		propagation = (typeof propagation == "undefined") ? true : (propagation===true) ? true : false;
		

		if (!self._eventQueues[eventName]) {
			self._eventQueues[eventName] = [];
		}

		self._eventQueues[eventName].push({
			name : eventName,
			throwEvent : callback,
			propagation : true
		});

		self["on"+eventName] = function(e){
			for(var i in self._eventQueues[eventName]){
				self._eventQueues[eventName][i].throwEvent.call(self, e);
				if (self._eventQueues[eventName][i].propagation===false){
					break;
				}
			}
		};

	}
});



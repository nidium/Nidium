/* ------------------------+------------- */
/* Native Framework 2.0    | Falcon Build */
/* ------------------------+------------- */
/* (c) 2013 nidium.com - Vincent Fontaine */
/* -------------------------------------- */

/* -------------------------------------------------------------------------- */
/* NSS PROPERTIES                                                             */
/* -------------------------------------------------------------------------- */

document.nss.add({
	"UIKnob" : {
		width : 96,
		height : 96,
		backgroundImage : "private://assets/knobs/96x96.png",

		value : 0,
		min : 0,
		max : 100,
		value : 0,
		releaseTime : 0,
		releaseValue : 0
	}
});

/* -------------------------------------------------------------------------- */
/* ELEMENT DEFINITION                                                         */
/* -------------------------------------------------------------------------- */

Native.elements.extend("UISprite").export("UIKnob", {
	onAdoption : function(){
		/* overide UISprite onAdoption with nothing to do */
	},

	init : function(){
		var self = this,
			o = this.options;

		this.dragging = false;
		this.loop = false;
		this.stop();

		this.__onload = function(){
			this.fireEvent("load", {});
		};

		this.getValueStep = function(){
			return (this.max - this.min)/this.frames;
		};

		this.releaseToValue = function(value){
			if (this.value === value) return false;
			this.setValue(
				value,
				this.releaseTime,
				function(){
					self.fireEvent("complete", {
						value : this.value
					});
				}, 
				Math.physics.quadOut
			);
		};

		this.increaseValueByStep = function(steps){
			var inc = this.getValueStep();
			this.value += steps * inc;
			
			if (this._oldvalue === this.value) return false;

			this._oldvalue = this.value;
			this.fireEvent("change", {
				value : this.value
			});
		};

		this.addEventListener("mousewheel", function(e){
			if (!this.loaded) return false;

			var now = +new Date();

			if (!this.__lastevent__) {
				this.__ellapsed__ = 0;
			} else {
				this.__ellapsed__ = now - this.__lastevent__;
			}

			//this.stopCurrentAnimation();
			this.increaseValueByStep(e.yrel);

			if (this.releaseTime>0 && this.__ellapsed__ < 250){
				this.finishCurrentAnimations("frame");

				clearTimeout(this.__timer__);
				this.__timer__ = setTimeout(function(){
					self.releaseToValue(self.releaseValue);
				}, 250);
			}

			this.__lastevent__ = now;

		}, true);

		this.addEventListener("dragstart", function(e){
			if (!this.loaded) return false;
			this.dragging = true;
			//this.stopCurrentAnimation();
		}, true);

		this.addEventListener("drag", function(e){
			if (this.dragging !== true || !this.loaded) return false;
			this.increaseValueByStep(-e.yrel);
		}, true);

		this.addEventListener("dragend", function(e){
			if (!this.loaded) return false;
			this.dragging = false;

			if (this.releaseTime>0) {
				this.releaseToValue(this.releaseValue);
			}

			this.fireEvent("complete", {
				value : this.value
			});
		}, true);
	}
});

/* ------------------------+------------- */
/* Native Framework 2.0    | Falcon Build */
/* ------------------------+------------- */
/* (c) 2013 nidium.com - Vincent Fontaine */
/* -------------------------------------- */

Native.elements.export("UISliderController", {
	public : {
		vertical : {
			value : function(){
				return OptionalBoolean(this.options.vertical, false);
			},

			set : function(value){
				this.changeOrientation(value);
			}
		}
	},

	default : {
		canReceiveFocus : true,
		color : "#3388dd",
		value : 0,
		min : 0,
		max : 100,
		cursor : "pointer",
		boxColor : null,
		progressBarColor : null,
		splitColor : null
	},

	init : function(){
		var self = this,
			o = this.options;

		NDMElement.defineDynamicProperties(this, {
			canReceiveFocus : true,
			color : OptionalValue(o.color, "#3388dd"),
			value : OptionalNumber(o.value, 0),
			min : OptionalNumber(o.min, 0),
			max : OptionalNumber(o.max, 100),
			cursor : "pointer",

			boxColor : OptionalValue(o.boxColor, false),
			progressBarColor : OptionalValue(o.progressBarColor, false),
			splitColor : OptionalValue(o.splitColor, false)
		});

		if (this.vertical){
			this.width = OptionalNumber(o.width, 12);
			this.height = OptionalNumber(o.height, 100);
		} else {
			this.width = OptionalNumber(o.width, 100);
			this.height = OptionalNumber(o.height, 12);
		}

		this.changeOrientation = function(vertical){
			var tmpWidth = this._width;
			this.width = this._height;
			this.height = tmpWidth;

			if (vertical){
				this.knob.left = 0;
			} else {
				this.knob.top = 0;
			}
			this.setValue(this.value);
		};

		this.knob = this.add("UISliderKnob", {
			left : this.vertical ? 0 : -this.height/2,
			top : this.vertical ? this.height - this.width/2 : 0,
			width : this.vertical ? this.width : this.height,
			height : this.vertical ? this.width : this.height,
			background : this.color,
			cursor : "drag"
		});

		this.addEventListener("mousedown", function(e){
			var k = this.knob,
				v = self.vertical,

				start = v ? k.top : k.left,
				delta = v ? (e.y - this.__top - k.height/2)
						  : (e.x - this.__left - k.width/2),

				property = v ? "top" : "left";

			k.finishCurrentAnimations(property);
			k.animate(property, start, delta, 200,
				function(){
					self.fireEvent("complete", self.value);
				},

				Math.physics.quadOut,

				function(pixelValue){
					self.setKnobPosition(pixelValue);
				}
			);

			self.draggingSlider = true;
		}, false);

		this.addEventListener("drag", function(e){
			var k = this.knob,
				v = self.vertical,

				start = v ? k.top : k.left,
				delta = v ? (e.y - this.__top - k.height/2)
						  : (e.x - this.__left - k.width/2),

				property = v ? "top" : "left";

			k.finishCurrentAnimations(property);
			self.setKnobPosition(delta);

			self.draggingSlider = true;
		}, false);

		this.addEventListener("mouseup", function(e){
			self.draggingSlider = false;
			e.stopPropagation();
		}, false);

		this.knob.addEventListener("mousedown", function(e){
			self.draggingSlider = true;
			e.stopPropagation();
		}, false);

		this.knob.addEventListener("mouseup", function(e){
			self.draggingSlider = false;
			e.stopPropagation();
		}, false);

		this.knob.addEventListener("dragstart", function(e){
			self.draggingSlider = true;
			e.stopPropagation();
		}, false);

		this.knob.addEventListener("drag", function(e){
			var nx = e.x - self.__left - this.width/2,
				ny = e.y - self.__top - this.height/2,
				pixelValue = self.vertical ? ny : nx;

			self.draggingSlider = true;
			self.setKnobPosition(pixelValue);
		});

		this.knob.addEventListener("dragend", function(){
			self.draggingSlider = false;
			self.fireEvent("complete", self.value);
		}, false);

		this.addEventListener("mousewheel", function(e){
			var k = this.knob,
				d = 0,
				delta = 0,
				pixelValue = 0;

			if (this.vertical){
				d = e.yrel != 0 ? e.yrel : 0;
				delta = 1 + (-d-1);
				pixelValue = k.top + delta;
			} else {
				d = e.xrel != 0 ? e.xrel : 0;
				delta = 1 + (-d-1);
				pixelValue = k.left + delta;
			}
			self.setKnobPosition(pixelValue);

			if (this.vertical && e.xrel!=0 || !this.vertical && e.yrel!=0) {
				e.forcePropagation();
			} else {
				e.stopPropagation();
			}
		}, true);

		this.setKnobPosition = function(pixelValue){
			var k = this.knob,
				d = this.max - this.min,
				h = this.height,
				w = this.width,

				kh = k.height/2,
				kw = k.width/2;

			if (this.vertical){
				k.top = pixelValue.bound(-kh, h-kh);
				this.pixelValue = k.top + kh;
				this.value = this.max - this.pixelValue * d/h;
			} else {
				k.left = pixelValue.bound(-kw, w-kw);
				this.pixelValue = k.left + kw;
				this.value = this.pixelValue * d/w + this.min;
			}
			this.fireEvent("change", {
				value : this.value
			});
		};

		this.animateKnob = function(property, start, end, duration, callback){
			var ease = this.options.ease ? this.options.ease 
										 : Math.physics.expoIn;

			if (this.moving == true) {
				this.moving = false;
				this.knob.finishCurrentAnimations(property);
				this.setKnobPosition(end);
			} else {
				this.moving = true;

				this.knob.animate(property, start, end, duration,
					function(){
						self.moving = false;
						if (typeof(callback) == "function"){
							callback.call(self, self.value);
						}
					},

					ease,

					function(k){
						self.setKnobPosition(k);
					}
				);
			}
		};

		this.setValue = function(value, duration, callback){
			var oldValue = this.value,
				k = this.knob,
				d = this.max - this.min,
				h = this.height,
				w = this.width,
				kh = k.height/2,
				kw = k.width/2;

			if (this.draggingSlider) return false;

			this.value = Math.max(Math.min(this.max, value), this.min);

			if (this.vertical){
				var pxv = (this.max - this._value)*h/d - kh; // pixelview
				if (duration && duration>0) {
					this.animateKnob("top", k.top, pxv, duration, callback);
				} else {
					this.setKnobPosition(pxv);
				}
			} else {
				var pxv = (this._value - this.min)*w/d - kw;
				if (duration && duration>0) {
					this.animateKnob("left", k.left, pxv, duration, callback);
				} else {
					this.setKnobPosition(pxv);
				}
			}

		};

		this.setValue(this.value);

	},

	draw : function(context){
		var	params = this.getDrawingBounds(),
			x, y, 
			w, h;

		if (this.boxColor){
			context.roundbox(
				params.x, params.y, 
				params.w, params.h, 
				this.radius, this.boxColor, false
			);
		}

		if (this.vertical){
			w = Math.round(params.w/3);
			h = params.h;
			x = Math.floor(params.x + w);
			y = params.y;
		} else {
			w = params.w;
			h = Math.round(params.h/3);
			x = params.x;
			y = Math.floor(params.y + h);
		}

		context.setShadow(0, 1, 1, "rgba(255, 255, 255, 0.10)");
		context.roundbox(x, y, w, h, this.radius, this.background, false);
		context.setShadow(0, 0, 0);

		if (this.background){
			context.roundbox(
				x, y, 
				w, h, 
				this.radius, "rgba(0, 0, 0, 0.25)", false
			);
		}

		if (this.progressBarColor){
			var ga = context.globalAlpha;
			context.globalAlpha = 0.8;
			context.setShadow(0, 0, 4, this.progressBarColor);
			context.globalAlpha = ga;
			if (this.vertical){
				context.roundbox(
					x, y + this.pixelValue, 
					w, h - this.pixelValue, 
					this.radius, this.progressBarColor, false
				);
			} else {
				context.roundbox(
					x, y, 
					this.pixelValue, h,
					this.radius, this.progressBarColor, false
				);
			}
			context.setShadow(0, 0, 0);
		}

		if (this.splitColor){
			context.strokeStyle = this.splitColor;
			context.beginPath();
			if (this.vertical){
				for (var i=1; i<h-1; i+=4){
					context.moveTo(x, y+i);
					context.lineTo(x+w, y+i);
				}
			} else {
				for (var i=1; i<w-1; i+=4){
					context.moveTo(x+i, y);
					context.lineTo(x+i, y+h);
				}
			}
			context.stroke();
		}

	}
});
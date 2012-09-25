/* -------------------------- */
/* Native (@) 2012 Stight.com */
/* -------------------------- */

UIElement.extend("UISliderController", {
	init : function(){
		var self = this;

		this.flags._canReceiveFocus = true;
		this.name = this.options.name || "Default";

		this.value = this.options.value && typeof(this.options.value)=='number' ? this.options.value : 0;
		this.min = this.options.min && typeof(this.options.min)=='number' ? this.options.min : 0;
		this.max = this.options.max && typeof(this.options.max)=='number' ? this.options.max : 100;

		this.color = this.options.color || "#3388dd";
		this.boxColor = (this.options.boxColor) ? this.options.boxColor : false;
		this.progressBarColor = this.options.progressBarColor || false;
		this.splitColor = this.options.splitColor || false;

		this.vertical = this.options.vertical || false;

		if (this.vertical){
			this.w = this.options.w || 12;
			this.h = this.options.h || 100;
		} else {
			this.w = this.options.w || 100;
			this.h = this.options.h || 12;
		}

		if (this.vertical) {
			this.knob = this.add("UISliderKnob", {
				x : 0,
				y : this.h-this.w/2,
				w : this.w,
				h : this.w,
				background : this.color
			});
		} else {
			this.knob = this.add("UISliderKnob", {
				x : -this.h/2,
				y : 0, 
				w : this.h,
				h : this.h,
				background : this.color
			});
		}

		this.addEventListener("mousedown", function(e){
			if (self.vertical){
				var start = this.knob.y;
					y = e.y - this.__y - this.knob.h/2;
				this.knob.animate("top", start, y, 200,
					function(){

					},

					FXAnimation.easeOutQuad,

					function(ky){
						self.setKnobPosition(ky);
					}
				);
				self.draggingSlider = true;
			} else {
				var start = this.knob.x;
						x = e.x - this.__x - this.knob.w/2;
					this.knob.animate("left", start, x, 200,
						function(){

						},

						FXAnimation.easeOutQuad,

						function(kx){
							self.setKnobPosition(kx);
						}
					);
					self.draggingSlider = true;
			}

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
			self.draggingSlider = true;
			var position = self.vertical ? this.top + e.yrel : this.left + e.xrel;
			self.setKnobPosition(position);
		});

		this.knob.addEventListener("dragend", function(){
			self.draggingSlider = false;
		}, false);

		this.addEventListener("mousewheel", function(e){
			var d = e.yrel !=0 ? e.yrel : e.xrel !=0 ? e.xrel : 0,
				delta = 1 + (-d-1);
			if (self.vertical){
				self.setKnobPosition(this.knob.top + delta);
			} else {
				self.setKnobPosition(this.knob.left + delta);
			}
		}, false);

		this.setKnobPosition = function(pixelValue){
			var k = this.knob,
				d = this.max - this.min;

			if (this.vertical){
				k.top = pixelValue;
				k.top = Math.min(Math.max(-k.h/2, k.top), this.h - k.h/2);
				this.pixelValue = k.y + k.h/2;
				this.value = this.max - this.pixelValue*d/this.h;
			} else {
				k.left = pixelValue;
				k.left = Math.min(Math.max(-k.w/2, k.left), this.w - k.w/2);
				this.pixelValue = k.x + k.w/2;
				this.value = this.pixelValue*d/this.w + this.min;
			}
			this.fireEvent("change", this.value);
		};

		this.setValue = function(value, duration){
			var oldValue = this.value,
				d = this.max - this.min;

			if (this.draggingSlider) return false;

			this.value = Math.max(Math.min(this.max, value), this.min);

			if (this.vertical){
				this.pixelValue = (this.max - this.value)*this.h/d - this.knob.h/2;
				if (duration && duration>0) {
					var start = this.knob.y,
						y = this.pixelValue;

					this.knob.animate("top", start, y, duration,
						function(){

						},

						self.options.ease ? self.options.ease : FXAnimation.easeInExpo,

						function(ky){
							self.setKnobPosition(ky);
						}
					);
				} else {
					this.setKnobPosition(this.pixelValue);
				}
			} else {
				this.pixelValue = (this.value - this.min)*this.w/d - this.knob.w/2;

				if (duration && duration>0) {
					var start = this.knob.x,
						x = this.pixelValue;

					this.knob.animate("left", start, x, duration,
						function(){

						},

						self.options.ease ? self.options.ease : FXAnimation.easeInExpo,

						function(kx){
							self.setKnobPosition(kx);
						}
					);
				} else {
					this.setKnobPosition(this.pixelValue);
				}
			}


		};

		this.setValue(this.value, 800);

	},

	draw : function(){
		var params = {
				x : this._x,
				y : this._y,
				w : this.w,
				h : this.h
			},
			x, y, w, h;

		if (this.boxColor){
			canvas.roundbox(params.x, params.y, params.w, params.h, this.radius, this.boxColor, false);
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

		canvas.setShadow(0, 1, 1, "rgba(255, 255, 255, 0.10)");
		canvas.roundbox(x, y, w, h, this.radius, this.background, false);
		canvas.setShadow(0, 0, 0);

		if (this.background){
			canvas.roundbox(x, y, w, h, this.radius, "rgba(0, 0, 0, 0.25)", false);
		}

		if (this.progressBarColor){
			var ga = canvas.globalAlpha;
			canvas.globalAlpha = 0.8;
			canvas.setShadow(0, 0, 4, this.progressBarColor);
			canvas.globalAlpha = ga;
			if (this.vertical){
				canvas.roundbox(x, y + this.pixelValue, w, h-this.pixelValue, this.radius, this.progressBarColor, false);
			} else {
				canvas.roundbox(x, y, this.pixelValue, h, this.radius, this.progressBarColor, false);
			}
			canvas.setShadow(0, 0, 0);
		}

		if (this.splitColor){
			canvas.strokeStyle = this.splitColor;
			canvas.beginPath();
			if (this.vertical){
				for (var i=0; i<h; i+=4){
					canvas.moveTo(x, y+i);
					canvas.lineTo(x+w, y+i);
				}
			} else {
				for (var i=0; i<w; i+=4){
					canvas.moveTo(x+i, y);
					canvas.lineTo(x+i, y+h);
				}
			}
			canvas.stroke();
		}

	}
});
/* -------------------------- */
/* Native (@) 2012 Stight.com */
/* -------------------------- */

Native.elements.export("UISliderController", {
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

		this.labelBackground = this.options.labelBackground || false;
		this.labelColor = this.options.labelColor || false;
		this.labelOffset = -24;
		this.labelWidth = this.options.labelWidth && typeof(this.options.labelWidth)=='number' ? this.options.labelWidth : 36;

		this.displayLabel = this.options.displayLabel ? true : false;
		this.labelPrefix = this.options.labelPrefix ? this.options.labelPrefix : '';
		this.labelSuffix = this.options.labelSuffix ? this.options.labelSuffix : '';



		this.vertical = this.options.vertical || false;

		if (this.vertical){
			this.w = this.options.w || 12;
			this.h = this.options.h || 100;
		} else {
			this.w = this.options.w || 100;
			this.h = this.options.h || 12;
		}

		this.knob = this.add("UISliderKnob", {
			x : this.vertical ? 0 : -this.h/2,
			y : this.vertical ? this.h-this.w/2 : 0,
			w : this.vertical ? this.w : this.h,
			h : this.vertical ? this.w : this.h,
			background : this.color
		});

		this.addEventListener("mousedown", function(e){
			var start = self.vertical ? this.knob.y : this.knob.x,
				delta = self.vertical ?(e.y - this.__y - this.knob.h/2) : (e.x - this.__x - this.knob.w/2),
				property = self.vertical ? "top" : "left";

			this.knob.animate(property, start, delta, 200,
				function(){

				},

				FXAnimation.easeOutQuad,

				function(pixelValue){
					self.setKnobPosition(pixelValue);
				}
			);

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
			self.draggingSlider = true;
			var position = self.vertical ? this.top + e.yrel : this.left + e.xrel;
			self.setKnobPosition(position);
		});

		this.knob.addEventListener("dragend", function(){
			self.draggingSlider = false;
			self.fireEvent("complete", self.value);
		}, false);

		this.addEventListener("mousewheel", function(e){
			var d = e.yrel !=0 ? e.yrel : e.xrel !=0 ? e.xrel : 0,
				delta = 1 + (-d-1);

			self.setKnobPosition( self.vertical ? this.knob.top + delta : this.knob.left + delta);
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

		this.setValue = function(value, duration, callback){
			var oldValue = this.value,
				d = this.max - this.min;

			if (this.draggingSlider) return false;

			this.value = Math.max(Math.min(this.max, value), this.min);
			this.moving = true;

			if (this.vertical){
				this.pixelValue = (this.max - this.value)*this.h/d - this.knob.h/2;
				if (duration && duration>0) {
					var start = this.knob.y,
						y = this.pixelValue;

					this.knob.animate("top", start, y, duration,
						function(){
							self.moving = false;
							typeof(callback)=="function" && callback.call(self, self.value);
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
							self.moving = false;
							typeof(callback)=="function" && callback.call(self, self.value);
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

		this.setValue(this.value, 400);

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

		if (!this.vertical && this.displayLabel){
			var kx = this.knob.__x,
				ky = this.knob.__y,
				kw = this.knob.__w,
				kh = this.knob.__h,
				vOffset = 2+this.lineHeight/2,
				tx = mx = 0,
				textWidth = 0,
				value = this.labelPrefix + Math.round(this.value*10)/10 + this.labelSuffix;


			canvas.setFontSize(this.fontSize);
			textWidth = canvas.measureText(value);

			tx = Math.round(kx + kw/2 - this.labelWidth/2);
			mx = (this.labelWidth - textWidth)/2;

			if (this.labelBackground){
				canvas.roundbox(tx, ky+this.labelOffset, this.labelWidth, this.lineHeight, 8, this.labelBackground, false);
			}

			if (this.labelColor){
				canvas.setColor(this.labelColor);
				canvas.fillText(value, tx+mx, ky - vOffset);
			}
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
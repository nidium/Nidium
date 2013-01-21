/* ------------------------+------------- */
/* Native Framework 2.0    | Falcon Build */
/* ------------------------+------------- */
/* (c) 2013 Stight.com - Vincent Fontaine */
/* -------------------------------------- */

Native.elements.export("UISliderController", {
	init : function(){
		var self = this,
			o = this.options;

		this.canReceiveFocus = true;

		this.color = OptionalValue(o.color, "#3388dd");

		DOMElement.definePublicProperties(this, {
			value : OptionalNumber(o.value, 0),
			min : OptionalNumber(o.min, 0),
			max : OptionalNumber(o.max, 100),

			boxColor : OptionalValue(o.boxColor, false),
			progressBarColor : OptionalValue(o.progressBarColor, false),
			splitColor : OptionalValue(o.splitColor, false),

			labelBackground : OptionalValue(o.labelBackground, false),
			labelColor : OptionalValue(o.labelColor, false),
			labelOffset : -24,
			labelWidth : OptionalNumber(o.labelWidth, 36)
		});

		this.displayLabel = OptionalBoolean(o.displayLabel, false);
		this.labelPrefix = OptionalString(o.labelPrefix, '');
		this.labelSuffix = OptionalString(o.labelSuffix, '');

		this.vertical = OptionalBoolean(o.vertical, false);

		if (this.vertical){
			this.width = OptionalNumber(o.width, 12);
			this.height = OptionalNumber(o.height, 100);
		} else {
			this.width = OptionalNumber(o.width, 100);
			this.height = OptionalNumber(o.height, 12);
		}

		this.knob = this.add("UISliderKnob", {
			left : this.vertical ? 0 : -this.height/2,
			top : this.vertical ? this.height - this.width/2 : 0,
			width : this.vertical ? this.width : this.height,
			height : this.vertical ? this.width : this.height,
			background : this.color
		});

		this.addEventListener("mousedown", function(e){
			var k = this.knob,
				v = self.vertical,

				start = v ? k.top : k.left,
				delta = v ? (e.y - this.__top - k.height/2)
						  : (e.x - this.__left - k.width/2),

				property = v ? "top" : "left";

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
				d = e.yrel !=0 ? e.yrel : e.xrel !=0 ? e.xrel : 0,
				delta = 1 + (-d-1),
				pixelValue = self.vertical ? k.top + delta : k.left + delta;

			self.setKnobPosition(pixelValue);
			e.forcePropagation();
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
			this.fireEvent("update", this.value);
		};

		this.setValue = function(value, duration, callback){
			var oldValue = this.value,
				d = this.max - this.min;

			if (this.draggingSlider) return false;

			this.value = Math.max(Math.min(this.max, value), this.min);
			this.moving = true;

			if (this.vertical){
				this.pixelValue = (this.max - this.value)*this.height/d - this.knob.height/2;
				if (duration && duration>0) {
					var start = this.knob.top,
						y = this.pixelValue;

					this.knob.animate("top", start, y, duration,
						function(){
							self.moving = false;
							if (typeof(callback)=="function"){
								callback.call(self, self.value);
							}
						},

						self.options.ease ? self.options.ease 
										  : Math.physics.expoIn,

						function(ky){
							self.setKnobPosition(ky);
						}
					);
				} else {
					this.setKnobPosition(this.pixelValue);
				}
			} else {
				this.pixelValue = (this.value - this.min)*this.width/d - this.knob.width/2;
				if (duration && duration>0) {
					var start = this.knob.left,
						x = this.pixelValue;

					this.knob.animate("left", start, x, duration,
						function(){
							self.moving = false;
							if (typeof(callback)=="function"){
								callback.call(self, self.value);
							}
						},

						self.options.ease ? self.options.ease 
										  : Math.physics.expoIn,

						function(kx){
							self.setKnobPosition(kx);
						}
					);
				} else {
					this.setKnobPosition(this.pixelValue);
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

		if (!this.vertical && this.displayLabel){
			var kx = this.knob.__left,
				ky = this.knob.__top,
				kw = this.knob._width,
				kh = this.knob._height,
				vOffset = 2+this.lineHeight/2,
				tx = mx = 0,
				textWidth = 0,

				value = this.labelPrefix + 
						Math.round(this.value*10)/10 + 
						this.labelSuffix;


			context.setFontSize(this.fontSize);
			textWidth = context.measureText(value);

			tx = Math.round(kx + kw/2 - this.labelWidth/2);
			mx = (this.labelWidth - textWidth)/2;

			if (this.labelBackground){
				context.roundbox(
					tx, ky+this.labelOffset, 
					this.labelWidth, this.lineHeight, 
					8, this.labelBackground, false
				);
			}

			if (this.labelColor){
				context.setColor(this.labelColor);
				context.fillText(value, tx+mx, ky - vOffset);
			}
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
					w, h-this.pixelValue, 
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
				for (var i=0; i<h; i+=4){
					context.moveTo(x, y+i);
					context.lineTo(x+w, y+i);
				}
			} else {
				for (var i=0; i<w; i+=4){
					context.moveTo(x+i, y);
					context.lineTo(x+i, y+h);
				}
			}
			context.stroke();
		}

	}
});
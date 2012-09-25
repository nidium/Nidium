/* -------------------------- */
/* Native (@) 2012 Stight.com */
/* -------------------------- */

UIElement.extend("UISliderController", {
	init : function(){
		var self = this;

		this.w = this.options.w || 100;
		this.h = this.options.h || 12;
		this.flags._canReceiveFocus = true;
		this.color = this.options.color || "#3388dd";
		this.name = this.options.name || "Default";

		this.value = this.options.value && typeof(this.options.value)=='number' ? this.options.value : 0;
		this.min = this.options.min && typeof(this.options.min)=='number' ? this.options.min : 0;
		this.max = this.options.max && typeof(this.options.max)=='number' ? this.options.max : 100;

		this.progressBarColor = this.options.progressBarColor || false;
		this.splitColor = this.options.splitColor || false;

		this.knob = this.add("UISliderKnob", {
			x : -this.h/2,
			y : 0, 
			w : this.h,
			h : this.h,
			background : this.color
		});

		this.addEventListener("mousedown", function(e){
			var start = this.knob.x;
				x = e.x - this.__x - this.knob.w/2;
			//this.setKnobPosition(x);
			this.knob.animate("left", start, x, 200,
				function(){

				},

				FXAnimation.easeOutQuad,

				function(kx){
					self.setKnobPosition(kx);
				}
			);
		}, false);

		this.knob.addEventListener("mousedown", function(e){
			e.stopPropagation();
		}, false);

		this.knob.addEventListener("dragstart", function(e){
			e.stopPropagation();
		}, false);

		this.knob.addEventListener("drag", function(e){
			self.setKnobPosition(this.left + e.xrel)
		});

		this.knob.addEventListener("dragend", function(){
		}, false);

		this.addEventListener("mousewheel", function(e){
			var d = e.yrel !=0 ? e.yrel : e.xrel !=0 ? e.xrel : 0,
				delta = 1 + (-d-1);
			self.setKnobPosition(this.knob.left + delta);
		}, false);

		this.setKnobPosition = function(pixelValue){
			var k = this.knob,
				d = this.max - this.min;

			k.left = pixelValue;
			k.left = Math.min(Math.max(-k.w/2, k.left), this.w - k.w/2);
			this.pixelValue = k.x + k.w/2;

			this.value = this.pixelValue*d/this.w + this.min;
			this.fireEvent("change", this.value);
		};

		this.setValue = function(value, duration){
			var d = this.max - this.min;

			this.value = Math.max(Math.min(this.max, value), this.min);
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

			h = Math.round(params.h/3),
			y = Math.floor(params.y + h);

		canvas.setShadow(0, 1, 1, "rgba(255, 255, 255, 0.10)");
		canvas.roundbox(params.x, y, params.w, h, this.radius, this.background, false);
		canvas.setShadow(0, 0, 0);

		if (this.background){
			canvas.roundbox(params.x, y, params.w, h, this.radius, "rgba(0, 0, 0, 0.25)", false);
		}

		if (this.progressBarColor){
			var ga = canvas.globalAlpha;
			canvas.globalAlpha = 0.8;
			canvas.setShadow(0, 0, 4, this.progressBarColor);
			canvas.globalAlpha = ga;
			canvas.roundbox(params.x, y, this.pixelValue, h, this.radius, this.progressBarColor, false);
			canvas.setShadow(0, 0, 0);
		}

		if (this.splitColor){
			canvas.strokeStyle = this.splitColor;
			canvas.beginPath();
			for (var i=0; i<params.w; i+=4){
				canvas.moveTo(params.x+i, y);
				canvas.lineTo(params.x+i, y+h);
			}
			canvas.stroke();
		}


	}
});
/* -------------------------- */
/* Native (@) 2013 Stight.com */
/* -------------------------- */

Native.elements.export("UIButton", {
	init : function(){
		var context = this.layer.context;
		context.setFontSize(this.fontSize);

		this.w = 10 + Math.round(context.measureText(this.label)) + 10;
		this.h = OptionalNumber(this.options.h, 22);
		this.radius = OptionalNumber(this.options.radius, 2);

		this.background = OptionalValue(this.options.background, "#2277E0");
		this.color = OptionalValue(this.options.color, "#ffffff");

		this.flags._canReceiveFocus = true;

		this.addEventListener("mousedown", function(e){
			this.selected = true;
		});

		this.addEventListener("mouseup", function(e){
			this.selected = false;
		});

		this.addEventListener("dragend", function(e){
			this.selected = false;
		});

		this.addEventListener("mouseover", function(e){
			this.hover = true;
		});

		this.addEventListener("mouseout", function(e){
			this.hover = false;
		});

	},

	draw : function(){
		var context = this.layer.context;

		context.setFontSize(this.fontSize);
		this.textWidth = Math.round(context.measureText(this.label));
		this.w = 10 + this.textWidth + 10;
		
		var params = {
				x : this._x,
				y : this._y,
				w : this.w,
				h : this.h
			},

			label = this.label,
			w = params.w,
			h = params.h,
			textOffsetX = 9,
			textOffsetY = (h-this.lineHeight)/2 + 4 + this.lineHeight/2,
			textShadow = '#000000';

		if (__ENABLE_BUTTON_SHADOWS__) {
			if (this.selected){
				context.setShadow(0, 2, 1, "rgba(255, 255, 255, 0.05)");
			} else {
				context.setShadow(0, 2, 3, "rgba(0, 0, 0, 0.5)");
			}
		}
		context.roundbox(
			params.x, params.y, 
			w, h, 
			this.radius, this.background, false
		);
		if (__ENABLE_BUTTON_SHADOWS__){
			context.setShadow(0, 0, 0);
		}


		if (this.selected){
			textOffsetY++;
			textShadow = "rgba(255, 255, 255, 0.15)";
		} else {
			textShadow = "rgba(0, 0, 0, 0.2)";
		}


		if (__ENABLE_GRADIENT_LAYERS__){
			var gradient = context.createLinearGradient(
				params.x, params.y,
				params.x, params.y+params.h
			);

			if (this.selected){
				gradient.addColorStop(0.00, 'rgba(0, 0, 0, 0.8)');
				gradient.addColorStop(0.25, 'rgba(0, 0, 0, 0.6)');
				gradient.addColorStop(1.00, 'rgba(0, 0, 0, 0.6)');
			} else {
				if (this.hover){
					gradient.addColorStop(0.00, 'rgba(255, 255, 255, 0.7)');
					gradient.addColorStop(0.50, 'rgba(255, 255, 255, 0.3)');
					gradient.addColorStop(0.50, 'rgba(255, 255, 255, 0.1)');
					gradient.addColorStop(0.80, 'rgba(0, 0, 0, 0.1)');
					gradient.addColorStop(1.00, 'rgba(0, 0, 0, 0.3)');
				} else {
					gradient.addColorStop(0.00, 'rgba(255, 255, 255, 0.4)');
					gradient.addColorStop(0.50, 'rgba(255, 255, 255, 0.1)');
					gradient.addColorStop(0.50, 'rgba(255, 255, 255, 0.0)');
					gradient.addColorStop(0.50, 'rgba(0, 0, 0, 0.0)');
					gradient.addColorStop(0.80, 'rgba(0, 0, 0, 0.1)');
					gradient.addColorStop(1.00, 'rgba(0, 0, 0, 0.3)');
				}

			}
		
			context.roundbox(
				params.x, params.y, 
				w, h, 
				this.radius, gradient, false
			);
		}

		//if (__TEXT_SHADOWS__) { context.setShadow(1, 1, 1, '#000000'); }
		context.setColor(this.color);
		context.fillText(label, params.x+textOffsetX, params.y+textOffsetY);
		//if (__TEXT_SHADOWS__){ context.setShadow(0, 0, 0); }


	}
});

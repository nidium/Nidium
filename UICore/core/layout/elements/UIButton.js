/* -------------------------- */
/* Native (@) 2012 Stight.com */
/* -------------------------- */

Native.elements.export("UIButton", {
	init : function(){
		canvas.setFontSize(this.fontSize);
		this.w = 10 + Math.round(canvas.measureText(this.label)) + 10;
		this.h = OptionalNumber(this.options.h, 22);
		this.radius = OptionalNumber(this.options.radius, 2);

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
		canvas.setFontSize(this.fontSize);
		this.textWidth = Math.round(canvas.measureText(this.label));
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
			textColor = '#e0e0e0',
			textShadow = '#000000';

		if (__ENABLE_BUTTON_SHADOWS__) {
			if (this.selected){
				canvas.setShadow(0, 2, 1, "rgba(255, 255, 255, 0.05)");
			} else {
				canvas.setShadow(0, 2, 3, "rgba(0, 0, 0, 0.5)");
			}
		}
		canvas.roundbox(params.x, params.y, w, h, this.radius, this.background, false);
		if (__ENABLE_BUTTON_SHADOWS__){
			canvas.setShadow(0, 0, 0);
		}


		if (this.selected){
			textOffsetY++;
			textColor = "rgba(255, 255, 255, 0.8)";
			textShadow = "rgba(255, 255, 255, 0.15)";
		} else {
			textColor = "rgba(255, 255, 255, 0.95)";
			textShadow = "rgba(0, 0, 0, 0.2)";
		}


		if (__ENABLE_GRADIENT_LAYERS__){
			var gdBackground = canvas.createLinearGradient(params.x, params.y, params.x, params.y+params.h);

			if (this.selected){
				gdBackground.addColorStop(0.00, 'rgba(0, 0, 0, 0.8)');
				gdBackground.addColorStop(0.25, 'rgba(0, 0, 0, 0.6)');
				gdBackground.addColorStop(1.00, 'rgba(0, 0, 0, 0.6)');
			} else {
				if (this.hover){
					gdBackground.addColorStop(0.00, 'rgba(255, 255, 255, 0.7)');
					gdBackground.addColorStop(0.50, 'rgba(255, 255, 255, 0.3)');
					gdBackground.addColorStop(0.50, 'rgba(255, 255, 255, 0.1)');
					gdBackground.addColorStop(0.80, 'rgba(0, 0, 0, 0.1)');
					gdBackground.addColorStop(1.00, 'rgba(0, 0, 0, 0.3)');
				} else {
					gdBackground.addColorStop(0.00, 'rgba(255, 255, 255, 0.4)');
					gdBackground.addColorStop(0.50, 'rgba(255, 255, 255, 0.1)');
					gdBackground.addColorStop(0.50, 'rgba(255, 255, 255, 0.0)');
					gdBackground.addColorStop(0.50, 'rgba(0, 0, 0, 0.0)');
					gdBackground.addColorStop(0.80, 'rgba(0, 0, 0, 0.1)');
					gdBackground.addColorStop(1.00, 'rgba(0, 0, 0, 0.3)');
				}

			}
		
			canvas.roundbox(params.x, params.y, w, h, this.radius, gdBackground, false);
		}

		canvas.setFontSize(this.fontSize);

		//if (__TEXT_SHADOWS__) { canvas.setShadow(1, 1, 1, '#000000'); }
		canvas.setColor(textColor);
		canvas.fillText(label, params.x+textOffsetX, params.y+textOffsetY);
		//if (__TEXT_SHADOWS__){ canvas.setShadow(0, 0, 0); }


	}
});

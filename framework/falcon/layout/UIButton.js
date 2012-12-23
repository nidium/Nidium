/* -------------------------- */
/* Native (@) 2013 Stight.com */
/* -------------------------- */

Native.elements.export("UIButton", {
	refresh : function(){
		var textWidth = Native.getTextWidth(
			this.label,
			this.fontSize,
			this.fontType
		);

		this.width = 10 + Math.round(textWidth) + 10;
	},

	init : function(){
		var o = this.options;

		this.height = OptionalNumber(o.height, 22);
		this.radius = OptionalNumber(o.radius, 2);

		this.background = OptionalValue(o.background, "#2277E0");
		this.color = OptionalValue(this.options.color, "#ffffff");

		this.canReceiveFocus = true;

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

	draw : function(context){
		var	params = this.getDrawingBounds(),

			textOffsetX = 9,
			textOffsetY = (params.h-this.lineHeight)/2 + 4 + this.lineHeight/2,
			textShadow = '#000000';

		context.setFontSize(this.fontSize);
		context.setFontType(this.fontType);

		if (__ENABLE_BUTTON_SHADOWS__) {
			if (this.selected){
				context.setShadow(0, 2, 1, "rgba(255, 255, 255, 0.06)");
			} else {
				context.setShadow(0, 2, 3, "rgba(0, 0, 0, 0.5)");
			}
		}
		context.roundbox(
			params.x, params.y,
			params.w, params.h, 
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
				params.w, params.h, 
				this.radius, gradient, false
			);
		}

		if (__ENABLE_TEXT_SHADOWS__) { context.setShadow(1, 1, 1, '#000000'); }
		context.setColor(this.color);
		context.fillText(
			this.label, 
			params.x+textOffsetX, params.y+textOffsetY
		);
		if (__ENABLE_TEXT_SHADOWS__) { context.setShadow(0, 0, 0); }
	}
});

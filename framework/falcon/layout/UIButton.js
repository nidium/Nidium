/* -------------------------- */
/* Native (@) 2013 Stight.com */
/* -------------------------- */

Native.elements.export("UIButton", {
	public : {
		label : {
			set : function(value){
				this.resizeElement();
			}
		},

		fontSize : {
			set : function(value){
				this.resizeElement();
			}
		},

		fontType : {
			set : function(value){
				this.resizeElement();
			}
		},

		paddingLeft : {
			set : function(value){
				this.resizeElement();
			}
		},
		
		paddingRight : {
			set : function(value){
				this.resizeElement();
			}
		}
	},

	init : function(){
		var o = this.options;

		this.setProperties({
			canReceiveFocus	: true,
			label			: OptionalString(o.label, "Button"),
			fontSize  		: OptionalNumber(o.fontSize, 11),
			fontType  		: OptionalString(o.fontType, "arial"),

			paddingLeft		: OptionalNumber(o.paddingLeft, 10),
			paddingRight	: OptionalNumber(o.paddingLeft, 10),

			height 			: OptionalNumber(o.height, 22),
			radius 			: OptionalNumber(o.radius, 2),
			background 		: OptionalValue(o.background, "#2277E0"),
			color 			: OptionalValue(o.color, "#ffffff")
		});

		this.resizeElement = function(){
			var textWidth = Native.getTextWidth(
				this.label,
				this.fontSize,
				this.fontType
			);
			this.width = this.paddingLeft + Math.round(textWidth) + this.paddingRight;
		};

		this.resizeElement();

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

			textOffsetX = this.paddingLeft-1,
			textOffsetY = (params.h-this.lineHeight)/2 + 4 + this.lineHeight/2,
			textShadow = '#000000';

		if (__ENABLE_BUTTON_SHADOWS__) {
			if (this.selected){
				context.setShadow(0, 1, 0.75, "rgba(255, 255, 255, 0.08)");
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

		context.setFontSize(this.fontSize);
		context.setFontType(this.fontType);

		context.setText(
			this.label,
			params.x+textOffsetX,
			params.y+textOffsetY,
			this.color,
			textShadow
		);
	}
});

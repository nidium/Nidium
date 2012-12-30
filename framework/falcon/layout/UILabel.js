/* -------------------------- */
/* Native (@) 2013 Stight.com */
/* -------------------------- */

Native.elements.export("UILabel", {
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
		}
	},

	init : function(){
		var o = this.options;

		this.setProperties({
			canReceiveFocus	: false,
			label			: OptionalString(o.label, ""),
			fontSize  		: OptionalNumber(o.fontSize, 11),
			fontType  		: OptionalString(o.fontType, "arial"),
			textAlign 		: OptionalAlign(o.textAlign, "left"),

			height 			: OptionalNumber(o.height, 18),
			radius 			: OptionalNumber(o.radius, 0),
			background 		: OptionalValue(o.background, ""),
			color 			: OptionalValue(o.color, "#222222")
		});

		this.resizeElement = function(){
			this._textWidth = Native.getTextWidth(
				this.label,
				this.fontSize,
				this.fontType
			);
		};

		this.resizeElement();

		this.width = OptionalNumber(
			o.width, this.paddingLeft + this._textWidth + this.paddingRight
		);
	},

	draw : function(context){
		var	params = this.getDrawingBounds(),
			textOffsetX = this.paddingLeft,
			textOffsetY = (params.h-this.lineHeight)/2 + 4 + this.lineHeight/2;

		var tx = params.x+textOffsetX,
			ty = params.y+textOffsetY;

		if (this.textAlign == "right") {
			tx = params.x + params.w - this._textWidth - this.paddingRight;
		}

		context.roundbox(
			params.x, params.y, 
			params.w, params.h, 
			this.radius, this.background, false
		);

		context.setFontSize(this.fontSize);
		context.setFontType(this.fontType);

		context.setText(
			this.label,
			params.x+textOffsetX,
			params.y+textOffsetY,
			this.color,
			"rgba(0, 0, 0, 0.4)"
		);

	}
});

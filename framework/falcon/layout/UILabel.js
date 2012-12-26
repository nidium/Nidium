/* -------------------------- */
/* Native (@) 2012 Stight.com */
/* -------------------------- */

Native.elements.export("UILabel", {
	refresh : function(){
		this._textWidth = Native.getTextWidth(
			this.label,
			this.fontSize,
			this.fontType
		);
	},

	init : function(){
		var o = this.options;
		
		this._textWidth = Native.getTextWidth(
			this.label,
			this.fontSize,
			this.fontType
		);

		this.paddingLeft = OptionalNumber(o.paddingLeft, 0);
		this.paddingRight = OptionalNumber(o.paddingLeft, 0);
		this.height = OptionalNumber(o.height, this.lineHeight);
		this.radius = OptionalNumber(o.radius, 0);
		this.background = OptionalValue(o.background, "");
		this.color = OptionalValue(o.color, "#222222");

		this.width = OptionalNumber(
			o.width, 
			this.paddingLeft + this._textWidth + this.paddingRight
		);

		this.canReceiveFocus = false;
	},

	draw : function(context){
		var	params = this.getDrawingBounds(),
			textOffsetX = this.paddingLeft,
			textOffsetY = (params.h-this.lineHeight)/2 + 4 + this.lineHeight/2;

		var tx = params.x+textOffsetX,
			ty = params.y+textOffsetY;

		if (this.textAlign == "right") {
			tx = params.x + params.w - _textWidth - this.paddingRight;
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

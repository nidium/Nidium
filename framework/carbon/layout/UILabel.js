/* -------------------------- */
/* Native (@) 2012 Stight.com */
/* -------------------------- */

Native.elements.export("UILabel", {
	init : function(){
		canvas.setFontSize(this.fontSize);

		this.textWidth = Math.round(canvas.measureText(this.label));
		this.paddingLeft = OptionalNumber(this.options.paddingLeft, 0);
		this.paddingRight = OptionalNumber(this.options.paddingLeft, 0);


		this.w = OptionalNumber(this.options.w , this.paddingLeft + this.textWidth + this.paddingRight);
		this.h = OptionalNumber(this.options.h, this.lineHeight);
		this.radius = OptionalNumber(this.options.radius, 0);

		this.background = OptionalValue(this.options.background, "");
		this.color = OptionalValue(this.options.color, "#222222");

		this.flags._canReceiveFocus = false;
	},

	draw : function(){
		canvas.setFontSize(this.fontSize);
		this.textWidth = Math.round(canvas.measureText(this.label));
		
		var params = {
				x : this._x,
				y : this._y,
				w : this.w,
				h : this.h
			},

			label = this.label,
			w = params.w,
			h = params.h,
			textOffsetX = this.paddingLeft,
			textOffsetY = (h-this.lineHeight)/2 + 4 + this.lineHeight/2,
			textShadow = '#000000';

		var tx = params.x+textOffsetX,
			ty = params.y+textOffsetY;

		if (this.textAlign == "right") {
			tx = params.x + params.w - this.textWidth - this.paddingRight;
		}

		canvas.roundbox(params.x, params.y, w, h, this.radius, this.background, false);

		if (__ENABLE_TEXT_SHADOWS__) { canvas.setShadow(1, 1, 1, '#000000'); }
		canvas.setColor(this.color);
		canvas.fillText(label, tx, ty);
		if (__ENABLE_TEXT_SHADOWS__){ canvas.setShadow(0, 0, 0); }

	}
});

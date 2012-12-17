/* -------------------------- */
/* Native (@) 2012 Stight.com */
/* -------------------------- */

Native.elements.export("UITextInput", {
	init : function(){
		var self = this;

		this.w = OptionalNumber(this.options.w, 140);;
		this.h = OptionalNumber(this.options.h, 24);
		this.lineHeight = OptionalNumber(this.options.lineHeight, this.h-4);
		this.background = OptionalValue(this.options.background, "#555555");
		this.color = OptionalValue(this.options.color, "#222222");

		this.textView = this.add("UIText", {
			x : 2,
			y : 2, 
			w : self.w - 4,
			h : self.h - 4,
			fontSize : self.fontSize,
			lineHeight : self.lineHeight,
			text : self.text,
			editable : true,
			background : "#f2f2f4"
		});

	},

	draw : function(){
		var context = this.layer.context,
			params = {
				x : this._x,
				y : this._y,
				w : this.w,
				h : this.h
			},
	
			radius = Math.max(4, this.radius);


		if (__ENABLE_BUTTON_SHADOWS__) {
			context.setShadow(0, 2, 3, "rgba(0, 0, 0, 0.5)");
		}
		context.roundbox(
			params.x, params.y, 
			params.w, params.h, 
			radius, this.background, false
		);

		if (__ENABLE_BUTTON_SHADOWS__){
			context.setShadow(0, 0, 0);
		}

		var gradient = context.createLinearGradient(
			params.x, params.y, 
			params.x, params.y+params.h
		);

		gradient.addColorStop(0.00, 'rgba(255, 255, 255, 0.20)');
		gradient.addColorStop(0.10, 'rgba(255, 255, 255, 0.05)');
		gradient.addColorStop(0.90, 'rgba(255, 255, 255, 0.00)');

		context.roundbox(
			params.x, params.y, 
			params.w, params.h, 
			radius, gradient, false
		);
	}
});
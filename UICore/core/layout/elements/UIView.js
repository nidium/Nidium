/* -------------------------- */
/* Native (@) 2012 Stight.com */
/* -------------------------- */

UIElement.extend("UIView", {
	init : function(){
	},

	draw : function(){
		var params = {
				x : this._x,
				y : this._y,
				w : this.w,
				h : this.h
			};

		if (this.shadowBlur !=0 ) {
			canvas.setShadow(0, 0, this.shadowBlur, "rgba(0, 0, 0, 0.5)");
		}

		canvas.roundbox(params.x, params.y, params.w, params.h, this.radius, this.background, false); // main view

		if (this.shadowBlur!=0) {
			canvas.setShadow(0, 0, 0);
		}

	}
});
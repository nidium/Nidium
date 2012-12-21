/* -------------------------- */
/* Native (@) 2013 Stight.com */
/* -------------------------- */

Native.elements.export("UIView", {
	init : function(){
		this.backgroundImage = OptionalValue(
			this.options.backgroundImage, 
			""
		);

		if (this.backgroundImage != "") {
			Native.getLocalImage(this, this.backgroundImage);
		}
	},

	draw : function(){
		var context = this.layer.context,
			params = {
				x : this._x,
				y : this._y,
				w : this.w,
				h : this.h
			};

		if (this.shadowBlur != 0) {
			context.setShadow(0, 0, this.shadowBlur, "rgba(0, 0, 0, 0.5)");
			context.roundbox(
				params.x, params.y, 
				params.w, params.h, 
				this.radius, this.background, false
			);
			context.setShadow(0, 0, 0);
		} else {
			context.roundbox(
				params.x, params.y, 
				params.w, params.h, 
				this.radius, this.background, false
			);
		}



		if (this._backgroundImage && this.backgroundImage != "") {
			context.save();
			context.roundbox(
				params.x, params.y, 
				params.w, params.h, 
				this.radius, this.background, false
			);
			context.clipbox(
				params.x, params.y,
				params.w, params.h,
				this.radius
			);
			context.clip();
			context.drawImage(
				this._backgroundImage,
				params.x, params.y
			);
			context.restore();
		}

	}
});
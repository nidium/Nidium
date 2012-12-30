/* -------------------------- */
/* Native (@) 2013 Stight.com */
/* -------------------------- */

Native.elements.export("UIView", {
	public : {
		backgroundImage : {
			value : function(){
				return OptionalString(this.options.backgroundImage, null);
			}
		}
	},

	init : function(){
		if (this.backgroundImage != '') {
			Native.getLocalImage(this, this.backgroundImage);
		}
	},

	draw : function(context){
		var	params = this.getDrawingBounds();

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

		if (this._cachedBackgroundImage && this.backgroundImage != "") {
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
				this._cachedBackgroundImage,
				params.x, params.y
			);
			context.restore();
		}

	}
});
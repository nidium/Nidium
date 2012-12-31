/* -------------------------- */
/* Native (@) 2013 Stight.com */
/* -------------------------- */

Native.elements.export("UIView", {
	public : {
		backgroundImage : {
			value : function(){
				return OptionalString(this.options.backgroundImage, '');
			},

			set : function(value){
				this.refreshBackgroundImage();
			}
		}
	},

	init : function(){
		var self = this;

		this.refreshBackgroundImage = function(){
			delete(self._cachedBackgroundImage);

			if (this.backgroundImage != '' && this.backgroundImage != null) {
				Native.loadImage(this.backgroundImage, function(img){
					self._cachedBackgroundImage = img;
					self._needRedraw = true;
					self.refresh();
				});
			}
		};

		this.refreshBackgroundImage();
	},

	draw : function(context){
		var	params = this.getDrawingBounds();

		if (this.shadowBlur != 0) {
			context.setShadow(
				this.shadowOffsetX,
				this.shadowOffsetY,
				this.shadowBlur,
				this.shadowColor
			);
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

		if (this._cachedBackgroundImage) {
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
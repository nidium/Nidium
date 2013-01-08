/* ------------------------+------------- */
/* Native Framework 2.0    | Falcon Build */
/* ------------------------+------------- */
/* (c) 2013 Stight.com - Vincent Fontaine */
/* -------------------------------------- */

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

		DOMElement.listeners.addDefault(this);

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
			DOMElement.draw.box(this, context, params);
			context.setShadow(0, 0, 0);
		} else {
			DOMElement.draw.box(this, context, params);
		}

		if (this._cachedBackgroundImage) {
			context.save();
				DOMElement.draw.box(this, context, params);
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
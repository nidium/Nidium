/* -------------------------- */
/* Native (@) 2012 Stight.com */
/* -------------------------- */

Native.getLocalImage = function(element, url, callback){
	var cb = OptionalCallback(callback, function(){});

	if (element._backgroundImage) {
		cb(element._backgroundImage);
	} else {
		var img = new Image();
		img.onload = function(){
			element._backgroundImage = img;
			cb(img);
		};
		img.src = url;
	}

};

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
		var params = {
				x : this._x,
				y : this._y,
				w : this.w,
				h : this.h
			};

		if (this.shadowBlur != 0) {
			canvas.setShadow(0, 0, this.shadowBlur, "rgba(0, 0, 0, 0.5)");
			canvas.roundbox(params.x, params.y, params.w, params.h, this.radius, this.background, false);
			canvas.setShadow(0, 0, 0);
		} else {
			canvas.roundbox(params.x, params.y, params.w, params.h, this.radius, this.background, false);
		}



		if (this._backgroundImage && this.backgroundImage != "") {
			canvas.save();
			canvas.roundbox(params.x, params.y, params.w, params.h, this.radius, this.background, false);
			canvas.clipbox(
				params.x, params.y,
				params.w, params.h,
				this.radius
			);
			canvas.clip();
			canvas.drawImage(
				this._backgroundImage,
				params.x, params.y
			);
			canvas.restore();
		}

	}
});
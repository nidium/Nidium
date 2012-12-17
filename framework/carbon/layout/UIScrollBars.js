/* -------------------------- */
/* Native (@) 2012 Stight.com */
/* -------------------------- */

Native.elements.export("UIVerticalScrollBar", {
	init : function(){
		this.w = 8;
		this.h = this.parent.h;
		this.x = this.parent.w - this.w;
		this.y = 0;
		this.opacity = 0;
	},

	draw : function(){
		var context = this.layer.context,
			params = {
				x : this._x,
				y : this._y,
				w : this.w,
				h : this.h
			};

		context.setColor("rgba(80, 80, 80, 0.4)");
		context.fillRect(params.x, params.y, params.w, params.h);
	}
});


Native.elements.export("UIVerticalScrollBarHandle", {
	init : function(){
		this.w = 8;
		this.h = this.parent.parent.h; // grandpere
		this.x = 0;
		this.y = 0;
	},

	draw : function(){
		var context = this.layer.context,
			UIView = this.parent.parent;

		UIView.scrollBarHeight = Math.max(UIView.content.height, UIView.h);

		if (UIView.h == UIView.scrollBarHeight) {
			this.parent.visible = false;
			this.visible = false;
		} else {
			this.parent.visible = true;
			this.visible = true;
		}

		var params = {
				x : this._x,
				y : this._y + (UIView.scroll.top * (this.parent.h / UIView.scrollBarHeight)),
				w : this.w,
				h : (UIView.h / UIView.scrollBarHeight) * this.parent.h
			};

		this._x = params.x;
		this._y = params.y;

		context.roundbox(
			params.x, params.y, 
			params.w, params.h, 
			5, "rgba(40, 40, 40, 0.80)", false
		);

	}
});


/* ------------------------+------------- */
/* Native Framework 2.0    | Falcon Build */
/* ------------------------+------------- */
/* (c) 2013 Stight.com - Vincent Fontaine */
/* -------------------------------------- */

Native.elements.export("UIScrollBar", {
	init : function(){
		var o = this.options;
		this.radius = 0;
		this.background = OptionalValue(o.background, "rgba(80, 80, 80, 0.20)");
		this.radius = 5;
		this.opacity = 0;
		this.visible = false;
	},

	draw : function(context){
		var	params = this.getDrawingBounds();
		DOMElement.draw.box(this, context, params);
	}
});

Native.elements.export("UIScrollBarHandle", {
	init : function(){
		var o = this.options;

		this.left = 0;
		this.top = 0;
		this.radius = 5;
		this.background = OptionalValue(o.background, "rgba(18, 18, 18, 0.80)");
		this.shadowBlur = 4;
		this.shadowColor = "rgba(255, 255, 255, 0.15)";
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
		}

		DOMElement.draw.box(this, context, params);
		context.setShadow(0, 0, 0);
	}
});


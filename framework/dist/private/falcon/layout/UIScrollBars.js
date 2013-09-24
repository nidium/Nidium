/* ------------------------+------------- */
/* Native Framework 2.0    | Falcon Build */
/* ------------------------+------------- */
/* (c) 2013 nidium.com - Vincent Fontaine */
/* -------------------------------------- */

Native.elements.export("UIScrollBar", {
	init : function(){
		var o = this.options;

		this.setProperties({
			background : OptionalValue(o.background, "rgba(80, 80, 80, 0.0)"),
			radius : OptionalNumber(o.radius, 5),
			opacity : OptionalNumber(o.opacity, 0),
			hidden : OptionalBoolean(o.hidden, false)
		});

		this.visible = false;
	},

	draw : function(context){
		if (this.hidden) return false;
		var	params = this.getDrawingBounds();
		NDMElement.draw.box(this, context, params);
	}
});

Native.elements.export("UIScrollBarHandle", {
	init : function(){
		var o = this.options;

		this.setProperties({
			left : 0,
			top : 0,
			radius : OptionalNumber(o.radius, 5),
			background : OptionalValue(o.background, "rgba(18, 18, 18, 0.80)"),
			shadowBlur : OptionalNumber(o.shadowBlur, 4),
			shadowColor : OptionalValue(
				o.shadowColor, 
				"rgba(255, 255, 255, 0.15)"
			)
		});
		this.hidden = this.parent ? this.parent.hidden : false;
	},

	draw : function(context){
		if (this.hidden) return false;
		var	params = this.getDrawingBounds();

		if (this.shadowBlur != 0) {
			context.setShadow(
				this.shadowOffsetX,
				this.shadowOffsetY,
				this.shadowBlur,
				this.shadowColor
			);
		}

		NDMElement.draw.box(this, context, params);
		context.setShadow(0, 0, 0);
	}
});


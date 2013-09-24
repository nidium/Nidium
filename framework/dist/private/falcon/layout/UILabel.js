/* ------------------------+------------- */
/* Native Framework 2.0    | Falcon Build */
/* ------------------------+------------- */
/* (c) 2013 nidium.com - Vincent Fontaine */
/* -------------------------------------- */

Native.elements.export("UILabel", {
	init : function(){
		var o = this.options;

		/* Element's Specific Dynamic Properties */
		NDMElement.definePublicProperties(this, {
			autosize : OptionalBoolean(o.autosize, true)
		});

		this.setProperties({
			canReceiveFocus	: false,
			label			: OptionalString(o.label, ""),
			fontSize  		: OptionalNumber(o.fontSize, 11),
			fontFamily  	: OptionalString(o.fontFamily, "arial"),
			textAlign 		: OptionalAlign(o.textAlign, "left"),

			textShadowOffsetX	: OptionalNumber(o.textShadowOffsetX, 1),
			textShadowOffsetY	: OptionalNumber(o.textShadowOffsetY, 1),
			textShadowBlur		: OptionalNumber(o.textShadowBlur, 1),
			textShadowColor 	: OptionalValue(
									o.textShadowColor,
									'rgba(0, 0, 0, 0.2)'
								),

			height 			: OptionalNumber(o.height, 18),
			radius 			: OptionalNumber(o.radius, 0),
			background 		: OptionalValue(o.background, ""),
			color 			: OptionalValue(o.color, "#222222")
		});
	},

	update : function(){
		if (this.autosize) {
			this.width = NDMElement.draw.getInnerTextWidth(this);
		}
	},

	draw : function(context){
		var	params = this.getDrawingBounds();

		NDMElement.draw.box(this, context, params);
		NDMElement.draw.label(this, context, params);
	}
});

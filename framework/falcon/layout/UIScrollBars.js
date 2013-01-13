/* ------------------------+------------- */
/* Native Framework 2.0    | Falcon Build */
/* ------------------------+------------- */
/* (c) 2013 Stight.com - Vincent Fontaine */
/* -------------------------------------- */

Native.elements.export("UIScrollBar", {
	init : function(){
		var o = this.options;
		this.radius = 0;
		this.background = OptionalValue(o.background, "rgba(80, 80, 80, 0.40)");
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
		this.background = OptionalValue(o.background, "rgba(40, 40, 40, 0.80)");
	},

	draw : function(context){
		var	params = this.getDrawingBounds();
		DOMElement.draw.box(this, context, params);
	}
});


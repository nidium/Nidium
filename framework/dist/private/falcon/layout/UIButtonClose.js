/* ------------------------+------------- */
/* Native Framework 2.0    | Falcon Build */
/* ------------------------+------------- */
/* (c) 2013 nidium.com - Vincent Fontaine */
/* -------------------------------------- */

Native.elements.export("UIButtonClose", {
	init : function(){
		var o = this.options;
		this.width = OptionalNumber(o.width, 10);
		this.height = OptionalNumber(o.height, 10);
		this.cursor = OptionalCursor(o.cursor, "pointer");

		DOMElement.listeners.addDefault(this);
	},

	draw : function(context){
		var	params = this.getDrawingBounds(),
			radius = this.width/2;

		if (this.background!='') {
			DOMElement.draw.circleBackground(this, context, params, radius);
		}

		var m = radius/1.6,
			x1 = params.x+m,
			y1 = params.y+m,
			x2 = params.x+params.w - m,
			y2 = params.y+params.h - m;

		context.strokeStyle = this.hover ? "#ffffff" : this.color;
		context.lineWidth = 2;

		context.beginPath();
		context.moveTo(x1, y1);
		context.lineTo(x2, y2);
		context.stroke();

		context.beginPath();
		context.moveTo(x1, y2);
		context.lineTo(x2, y1);
		context.stroke();
	}
});

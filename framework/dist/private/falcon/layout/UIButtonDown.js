/* ------------------------+------------- */
/* Native Framework 2.0    | Falcon Build */
/* ------------------------+------------- */
/* (c) 2013 Stight.com - Vincent Fontaine */
/* -------------------------------------- */

Native.elements.export("UIButtonDown", {
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
			console.log(this.background);
			DOMElement.draw.circleBackground(this, context, params, radius);
		}

		var m = radius/1.8,
			x1 = params.x+m - 0.25,
			y1 = params.y+m + 1,

			x2 = params.x+params.w - m + 0.4,
			y2 = y1,

			x3 = x1+(x2-x1)/2,
			y3 = params.y+params.h - m;

		if (this.hover) {
			context.globalAlpha = 0.8;
		}
		context.strokeStyle = this.color;
		context.setColor(this.color);
		if (this.hover) {
			context.globalAlpha = 1;
		}

		context.lineWidth = 0.5;
		context.lineCap = "round"; // default : butt
		context.lineJoin = "round"; // default : miter

		context.beginPath();
		context.moveTo(x1, y1);
		context.lineTo(x2, y2);
		context.lineTo(x3, y3);
		context.lineTo(x1, y1);
		context.stroke();
		context.fill();
	}
});

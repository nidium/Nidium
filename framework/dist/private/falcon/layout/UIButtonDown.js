/* ------------------------+------------- */
/* Native Framework 2.0    | Falcon Build */
/* ------------------------+------------- */
/* (c) 2013 nidium.com - Vincent Fontaine */
/* -------------------------------------- */

/* -------------------------------------------------------------------------- */
/* NSS PROPERTIES                                                             */
/* -------------------------------------------------------------------------- */

document.nss.add({
	"UIButtonDown" : {
		width : 10,
		height : 10,
		color : "rgba(0, 0, 0, 0.75)",
		background : "rgba(0, 0, 0, 0.3)",
		cursor : "arrow"
	},

	"UIButtonDown:hover" : {
		color : "#ffffff",
		cursor : "pointer"
	}
});

/* -------------------------------------------------------------------------- */
/* ELEMENT DEFINITION                                                         */
/* -------------------------------------------------------------------------- */

Native.elements.export("UIButtonDown", {
	init : function(){
		NDMElement.listeners.addDefault(this);
	},

	draw : function(context){
		var	params = this.getDrawingBounds(),
			radius = this.width/2;

		if (this.background) {
			NDMElement.draw.circleBackground(this, context, params, radius);
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

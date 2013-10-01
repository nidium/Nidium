/* ------------------------+------------- */
/* Native Framework 2.0    | Falcon Build */
/* ------------------------+------------- */
/* (c) 2013 nidium.com - Vincent Fontaine */
/* -------------------------------------- */

/* -------------------------------------------------------------------------- */
/* NSS PROPERTIES                                                             */
/* -------------------------------------------------------------------------- */

document.nss.add({
	"Icon" : {
		shape : "",
		variation : 0,
		width : 20,
		height : 20,
		cursor : "arrow"
	},

	"Icon:hover" : {
		cursor : "pointer"
	}
});

/* -------------------------------------------------------------------------- */
/* ELEMENT DEFINITION                                                         */
/* -------------------------------------------------------------------------- */


Native.elements.export("Icon", {
	init : function(){
		var o = this.options;

		/* Element's Specific Dynamic Properties */
		NDMElement.defineDynamicProperties(this, {
			shape : OptionalString(o.shape, ""),
			variation : OptionalNumber(o.variation, 0)
		});

		NDMElement.listeners.addDefault(this);
	},

	draw : function(context){
		var	params = this.getDrawingBounds(),
			radius = this.width/2,
			m = Math.round(radius/1.8);

		if (this.shape == "") return false;

		NDMElement.draw.box(this, context, params);

		context.strokeStyle = this.color;
		context.setColor(this.color);

		context.lineWidth = 0.5;
		context.lineCap = "round"; // default : butt
		context.lineJoin = "round"; // default : miter

		switch (this.shape) {
			case "play" : 
				var	x1 = params.x+m,
					y1 = params.y+m - 1,

					x2 = params.x+params.w - m + 0.5,
					y2 = params.y+params.h/2,

					x3 = x1,
					y3 = params.y+params.h - m + 1;


				context.beginPath();
				context.moveTo(x1, y1);
				context.lineTo(x2, y2);
				context.lineTo(x3, y3);
				context.lineTo(x1, y1);
				context.stroke();
				context.fill();
				break;

			case "pause" : 
				var	x1 = params.x+m,
					y1 = params.y+m - 1,

					x2 = params.x+params.w - m + 0.4,
					y2 = params.y+params.h/2;

				context.fillRect(x1, y1, m*0.5, params.h - 2*m+2);
				context.fillRect(x2-m*0.5, y1, m*0.5, params.h - 2*m+2);
				break;

			case "speaker" : 
				var	k = m*0.4,
					o = k*0.20,
					t = k*0.6,
					a = k*2,
					px = params.x,
					py = params.y,
					pw = params.w,
					ph = params.h,

					x1 = px+k,				y1 = py+m+t - 0.5,
					x2 = px+m+o,			y2 = y1,
					x3 = px+pw-m-a + 0.5,	y3 = py+k + 1,
					x4 = x3,				y4 = ph-k - 1,
					x5 = x2,				y5 = py+ph-m-t + 0.5,
					x6 = x1,				y6 = y5;

				context.beginPath();
				context.moveTo(x1, y1);
				context.lineTo(x2, y2);
				context.lineTo(x3, y3);
				context.lineTo(x4, y4);
				context.lineTo(x5, y5);
				context.lineTo(x6, y6);
				context.lineTo(x1, y1);
				context.stroke();
				context.fill();

				context.lineWidth = k*0.9;

				if (this.variation) {
					context.beginPath();
					context.arc(pw/2, ph/2, m-o-1, -0.7, 0.7);
					context.stroke();
					if (this.variation == 2) {
						context.beginPath();
						context.arc(pw/2, ph/2, m+k, -0.8, 0.8);
						context.stroke();
					}
				}

				break;

		}
	}
});

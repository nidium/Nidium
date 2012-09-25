/* -------------------------- */
/* Native (@) 2012 Stight.com */
/* -------------------------- */

canvas.implement({
	tabbox : function(x, y, w, h, radius, fill, stroke, lineWidth) {
		if (typeof fill == "undefined" ) {
			fill = false;
		}

		if (typeof stroke == "undefined" ) {
			stroke = false;
		}
		
		if (typeof radius === "undefined") {
			radius = 5;
		}
		
		this.lineWidth = lineWidth || 1;

		radius = 5;

		var x1 = x,
			y1 = y,
			x2 = x + w,
			y2 = y + h,

			t = 3.5 * radius,
			a = 0.7 * radius,
			b = 1.5 * radius,
			c = h / 3,
			m = 0.0;


		this.beginPath();

		this.moveTo(x1 + t, y1);

		this.lineTo(x2 - t, y1);
		this.quadraticCurveTo(x2 - b - a, y1,	x2 - b, y1 + c);
		this.quadraticCurveTo(x2 - b + a, y2,	x2, y2);  

		this.lineTo(x, y + h);
		this.quadraticCurveTo(x1 + b - a, y2,	x1 + b, y1 + c);
		this.quadraticCurveTo(x1 + b + a, y1,	x1 + t, y1);

		this.closePath();

		if (stroke) {
			this.strokeStyle = stroke;
			this.stroke();
		}

		if (fill) {
			this.setColor((fill=='') ? "rgba(0, 0, 0, 0.6)" : fill);
			this.fill();
		}        
	}
});


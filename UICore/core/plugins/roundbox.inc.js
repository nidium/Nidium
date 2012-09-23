/* -------------------------- */
/* Native (@) 2012 Stight.com */
/* -------------------------- */

canvas.implement({
	roundbox : function(x, y, width, height, radius, fill, stroke, lineWidth) {
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

		this.beginPath();

		this.moveTo(x + radius, y);

		this.lineTo(x + width - radius, y);
		this.quadraticCurveTo(x + width, y, x + width, y + radius);
		
		this.lineTo(x + width, y + height - radius);
		this.quadraticCurveTo(x + width, y + height, x + width - radius, y + height);

		this.lineTo(x + radius, y + height);
		this.quadraticCurveTo(x, y + height, x, y + height - radius);

		this.lineTo(x, y + radius);
		this.quadraticCurveTo(x, y, x+radius, y);

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
/* ------------------------+------------- */
/* Native Framework 2.0    | Falcon Build */
/* ------------------------+------------- */
/* (c) 2013 Stight.com - Vincent Fontaine */
/* -------------------------------------- */

Canvas.implement({
	roundbox : function(x, y, width, height, radius, fill, stroke, lineWidth) {
		if (!stroke && !fill) return false;

		fill = OptionalValue(fill, '');
		stroke = OptionalValue(stroke, '');
		radius = OptionalNumber(radius, 5);

		if (lineWidth !== null && lineWidth !== undefined) {
			this.lineWidth = OptionalNumber(lineWidth, 0);
		}

		if (fill) {
			this.setColor(fill);
			this.fillRect(x, y, width, height, radius);
		}

		if (stroke) {
			this.strokeStyle = stroke;
			this.strokeRect(x, y, width, height, radius);
		} 

	},

	clipbox : function(x, y, width, height, radius){
		radius = OptionalNumber(radius, 0);

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
	}

});
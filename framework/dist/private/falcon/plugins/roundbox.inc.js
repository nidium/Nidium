/* ------------------------+------------- */
/* Native Framework 2.0    | Falcon Build */
/* ------------------------+------------- */
/* (c) 2013 nidium.com - Vincent Fontaine */
/* -------------------------------------- */

Canvas.implement({
	roundbox : function(x, y, width, height, radius, fill, stroke, lineWidth){
		if (!stroke && !fill) return false;

		fill = OptionalValue(fill, '');
		stroke = OptionalValue(stroke, '');
		radius = OptionalNumber(radius, 5);

		if (fill){
			this.setColor(fill);
			this.fillRect(x, y, width, height, radius);
		}

		if (stroke && lineWidth){
			this.lineWidth = OptionalNumber(lineWidth, 0);
			this.strokeStyle = stroke;

			this.strokeRect(
				x-lineWidth/2,
				y-lineWidth/2,
				width + lineWidth,
				height + lineWidth,
				radius + lineWidth/2
			);
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
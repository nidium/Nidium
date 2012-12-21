/* -------------------------- */
/* Native (@) 2013 Stight.com */
/* -------------------------- */

Native.canvas.implement({
	setColor : function(color){
		this.fillStyle = color;
	},

	setFontSize : function(fontSize){
		this.fontSize = fontSize;
	},

	setShadow : function(x, y, b, c){
		this.shadowOffsetX = x;
		this.shadowOffsetY = y;
		this.shadowColor = c;
		this.shadowBlur = b;
	}
});

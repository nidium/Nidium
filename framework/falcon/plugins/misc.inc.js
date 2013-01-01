/* ------------------------+------------- */
/* Native Framework 2.0    | Falcon Build */
/* ------------------------+------------- */
/* (c) 2013 Stight.com - Vincent Fontaine */
/* -------------------------------------- */

Canvas.implement({
	setColor : function(color){
		this.fillStyle = color;
	},

	setFontSize : function(fontSize){
		this.fontSize = fontSize;
	},

	setFontType : function(fontType){
		this.fontType = fontType;
	},

	setShadow : function(x, y, b, c){
		this.shadowOffsetX = x;
		this.shadowOffsetY = y;
		this.shadowColor = c;
		this.shadowBlur = b;
	},

	setText : function(label, x, y, color, shadowColor){
		if (shadowColor && __ENABLE_TEXT_SHADOWS__) {
			this.setShadow(1, 1, 1, shadowColor);
		}

		this.setColor(color);
		this.fillText(label, x, y);

		if (shadowColor && __ENABLE_TEXT_SHADOWS__) {
			this.setShadow(0, 0, 0);
		}
	}
});

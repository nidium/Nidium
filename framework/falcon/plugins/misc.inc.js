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

	setShadow : function(sx, sy, sb, sc){
		this.shadowOffsetX = sx;
		this.shadowOffsetY = sy;
		this.shadowColor = sc;
		this.shadowBlur = sb;
	},

	setText : function(label, x, y, color, sx, sy, sb, sc){
		if (sb){
			this.setShadow(sx, sy, sb, sc);
		}

		this.setColor(color);
		this.fillText(label, x, y);
		this.setShadow(0, 0, 0);
	}
});

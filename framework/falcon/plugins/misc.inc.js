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
	},

	highlightLetters : function(letters, x, y, lineHeight){
		var c, nush, cx, cy, cw;
		for (var i=0; i<letters.length; i++){
			c = letters[i];
	 		nush = letters[i+1] ? 
	 						letters[i+1].position - c.position - c.width : 0;

		 	cx = x + c.position;
 			cy = y;
 			cw = c.width + nush;
 			if (c.selected){
				this.fillRect(cx, cy, cw, lineHeight);
			}
		}
	},

	drawLettersWithCaret : function(letters, x, y, lineHeight, vOffset, caretPosition, caretOpacity){
		var c, cx;
		for (var i=0; i<letters.length; i++){
			c = letters[i];
			cx = x + c.position;
			if (i == caretPosition){
				this.save();
				this.globalAlpha = caretOpacity;
				this.fillRect(Math.floor(cx), y - vOffset+1, 1, lineHeight-2);
				this.restore();
			}
			this.fillText(c.char, cx, y);
		}
	},

	drawLetters : function(letters, x, y){
		var c;
		for (var i=0; i<letters.length; i++){
			c = letters[i];
			this.fillText(c.char, x + c.position, y);
		}
	}
});

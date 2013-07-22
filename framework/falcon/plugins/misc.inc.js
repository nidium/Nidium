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

	drawCaret : function(letters, x, y, lineHeight, vOffset, caretPosition, caretOpacity){
		x = parseFloat(x);
		y = parseFloat(y);
		vOffset = parseFloat(vOffset);

		var i = caretPosition,
			cx = 0,
			cy = y - vOffset + 1;

		if (caretPosition == 0 && letters.length == 0) {
			/* empty text : caret to the begining of the line */
			cx = Math.floor(x);
		} else if (caretPosition < letters.length) {
			cx = Math.floor(x + letters[i].position);
		} else {
			/* caret after last char */
			cx = Math.floor(x + letters[i-1].position + letters[i-1].width);
		}

		this.save();
		this.globalAlpha = caretOpacity;
		this.fillStyle = "#888888";
		this.fillRect(cx, cy, 1, lineHeight - 2);
		this.restore();
	},

	drawLetters : function(letters, x, y){
		var c;
		for (var i=0; i<letters.length; i++){
			c = letters[i];
			this.fillText(c.char, x + c.position, y);
		}
	}
});

/* ------------------------+------------- */
/* Native Framework 2.0    | Falcon Build */
/* ------------------------+------------- */
/* (c) 2013 nidium.com - Vincent Fontaine */
/* -------------------------------------- */

var TextNode = function(text, className){
	text = String(text.replace(/\r\n/g, "\n"));
	if (text == '') return false;

	var options = {
		text : text,
		color : "#ffffff",
		class : String(className)
		//background : "rgba(255, 0, 0, 0.4)"
	};

	return new NDMElement("UITextNode", options, null);
};

Native.elements.export("UITextNode", {
	public : {
		text : {
			set : function(value){
				this.refreshElement();
			}
		},

		fontSize : {
			set : function(value){
				this.refreshElement();
			}
		},

		fontFamily : {
			set : function(value){
				this.refreshElement();
			}
		}
	},

	init : function(){
		var self = this,
			o = this.options;

		this.setProperties({
			canReceiveFocus	: false,
			text			: OptionalString(o.text, ""),
			fontSize  		: OptionalNumber(o.fontSize, 11),
			fontFamily  	: OptionalString(o.fontFamily, "arial"),
			textAlign 		: OptionalAlign(o.textAlign, "left"),

			textShadowOffsetX	: OptionalNumber(o.textShadowOffsetX, 1),
			textShadowOffsetY	: OptionalNumber(o.textShadowOffsetY, 1),
			textShadowBlur		: OptionalNumber(o.textShadowBlur, 1),

			textShadowColor 	: OptionalValue(
									o.textShadowColor,
									'rgba(0, 0, 0, 0.15)'
								),

			radius 			: OptionalNumber(o.radius, 0),
			background 		: OptionalValue(o.background, ""),
			color 			: OptionalValue(o.color, "#222222")
		});

		this.flags |= FLAG_TEXT_NODE | FLAG_FLOATING_NODE;
		this.textLines = [];
		this.linenum = 0;
		this.width = 0;
		this.height = 0;

		this.contentOffsetLeft = OptionalValue(o.contentOffsetLeft, []);
		this.contentOffsetRight = OptionalValue(o.contentOffsetRight, []);

		this.paddingLeft = 0;
		this.paddingRight = 0;
		this.paddingTop = 0;
		this.paddingBottom = 0;

		/* ------------------------------------------------------------------ */

		this.refreshElement = function(){
			this.width = 0;
			this.height = 0; 
		};

		this.inheritFromParent = function(){
			var p = this.parent;

			this._fontFamily = p.fontFamily;
			this._fontSize = p.fontSize;
			this._lineHeight = p.lineHeight;
			//this._color = p.color;
			this._textShadowOffsetX = p.textShadowOffsetX;
			this._textShadowOffsetY = p.textShadowOffsetY;
			this._textShadowBlur = p.textShadowBlur;
			this.textShadowColor = p.textShadowColor;
		};

		this.setDimensions = function(nb_lines, firstChar, lastChar){
			var fcpos = firstChar.position, // first char position
				fctop = firstChar.line; // absolute linenum in wholeText

			this.top = fctop * this.lineHeight;
			this.height = nb_lines * this.lineHeight;

			if (nb_lines && nb_lines <= 1) {

				this.left = fcpos;
				this.width = Math.round(
					lastChar.position + lastChar.width - fcpos
				);

			} else {
				this.left = 0;
				this.width = this.parent.maxWidth;
			}

		}

		this.updateElement = function(){
			var lines = this.textLines, // text matrix,
				nb_lines = lines.length, // number of node's lines of text
				nb_chars = lines[0].length, // nb chars in the first line

				firstLine = lines[0],
				firstChar = firstLine[0],
				lastChar = nb_chars>0 ? firstLine[nb_chars-1] : firstChar;

			this.inheritFromParent();
			this.setDimensions(nb_lines, firstChar, lastChar);
		};


		this.refreshElement();
	},

	onAdoption : function(parent){
		NDMElement.nodes.resetTextNodes(parent);
	},

	/* ---------------------------------------------------------------------- */
	/* - DRAW --------------------------------------------------------------- */
	/* ---------------------------------------------------------------------- */

	draw : function(context){
		var	params = this.getDrawingBounds(),

			x = params.x + this.paddingLeft,
			y = params.y + this.paddingTop,
			w = params.w - this.paddingRight - this.paddingLeft,
			h = params.h - this.paddingTop - this.paddingBottom;

		NDMElement.draw.box(this, context, params);

		context.setColor(this.color);
		context.fontSize = this.fontSize;
		context.fontFamily = this.fontFamily;

		context.setShadow(
			this.textShadowOffsetX,
			this.textShadowOffsetY,
			this.textShadowBlur,
			this.textShadowColor
		);

		NDMElement.draw.printMatrix(this, context, params);

		context.setShadow(0, 0, 0);
	}
});


NDMElement.draw.printMatrix = function(element, context, params){
	var vOffset = (element.lineHeight/2)+5,
		letters = element.letters;

	var lines = element.textLines;
		nb_lines = lines.length;

	for (var i=0; i<nb_lines; i++){
		var letters = lines[i],
			tx = params.x - element._left,
			ty = params.y + i * element.lineHeight + vOffset;

		//context.setColor("rgba(150, 150, 250, 0.25)");
		//context.highlightLetters(letters, tx, ty - vOffset, element.lineHeight);

		context.setColor(element.color);
		context.drawLetters(letters, tx, ty);

		if (element.fontWeight == "bold") {
			context.globalAlpha = 0.6;
			context.drawLetters(letters, tx, ty);
		} else if (element.fontWeight == "bolder") {
			context.globalAlpha = 0.4;
			context.drawLetters(letters, tx, ty);
			context.globalAlpha = 0.4;
			context.drawLetters(letters, tx, ty);
		}
	}

}

/* -------------------------------------------------------------------------- */
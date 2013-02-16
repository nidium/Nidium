/* ------------------------+------------- */
/* Native Framework 2.0    | Falcon Build */
/* ------------------------+------------- */
/* (c) 2013 Stight.com - Vincent Fontaine */
/* -------------------------------------- */

var TextNode = function(text){
	var options = {
		label : text,
		color : "#ffffff",
		background : "rgba(255, 0, 0, 0.4)"
	};

	var element = new DOMElement("UITextNode", options, null);
	element.flags |= FLAG_TEXT_NODE | FLAG_FLOATING_NODE;

	return element;
};

Native.elements.export("UITextNode", {
	public : {
		label : {
			set : function(value){
				this.refreshElement();
			}
		},

		fontSize : {
			set : function(value){
				this.refreshElement();
			}
		},

		fontType : {
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
			label			: OptionalString(o.label, ""),
			fontSize  		: OptionalNumber(o.fontSize, 11),
			fontType  		: OptionalString(o.fontType, "arial"),
			textAlign 		: OptionalAlign(o.textAlign, "left"),

			textShadowOffsetX	: OptionalNumber(o.textShadowOffsetX, 1),
			textShadowOffsetY	: OptionalNumber(o.textShadowOffsetY, 1),
			textShadowBlur		: OptionalNumber(o.textShadowBlur, 1),
			textShadowColor 	: OptionalValue(
									o.textShadowColor,
									'rgba(0, 0, 0, 0.1)'
								),

			radius 			: OptionalNumber(o.radius, 0),
			background 		: OptionalValue(o.background, ""),
			color 			: OptionalValue(o.color, "#222222")
		});

		this.height = OptionalNumber(o.lineHeight, 18);

		this.contentOffsetLeft = OptionalValue(o.contentOffsetLeft, []);
		this.contentOffsetRight = OptionalValue(o.contentOffsetRight, []);

		this.paddingLeft = 0;
		this.paddingRight = 0;
		this.paddingTop = 0;
		this.paddingBottom = 0;

		/* ------------------------------------------------------------------ */

		this.refreshElement = function(){
			var vOffset = (this.lineHeight/2)+5;

			this.textPixelWidth = Math.round(Native.getTextWidth(
				this._label,
				this._fontSize,
				this._fontType
			));
			this._textMatrix = getTextMatrixLines(this);
			
			var nblines = this._textMatrix.length;

			if (nblines<=1) {
				this.width = this.textPixelWidth;
				this.height = nblines * this.lineHeight;
			} else {
				this.width = this.parent ? 
								this.parent.width : this.textPixelWidth;

				this.height = nblines * this.lineHeight + vOffset;
			}

		};

		this.setLayout = function(contentOffsetLeft, contentOffsetRight){
			this.contentOffsetLeft = OptionalValue(contentOffsetLeft, []);
			this.contentOffsetRight = OptionalValue(contentOffsetRight, []);
		};

		this.refreshElement();

	},

	/* ---------------------------------------------------------------------- */
	/* - DRAW --------------------------------------------------------------- */
	/* ---------------------------------------------------------------------- */

	draw : function(context){
		var	params = this.getDrawingBounds(),

			x = params.x + this.paddingLeft,
			y = params.y + this.paddingTop,
			w = params.w - this.paddingRight - this.paddingLeft,
			h = params.h - this.paddingTop - this.paddingBottom,

			vOffset = (this.lineHeight/2)+5;

		DOMElement.draw.box(this, context, params);

		printTextMatrix(
			context,
			this._textMatrix, 
			null,
			x, y - this.parent ? this.parent.scrollTop : 0, 
			vOffset, 
			w, h, 
			params.y, 
			this.lineHeight,
			this.fontSize,
			this.fontType,
			this.color, 
			this.caretOpacity
		);

	}
});

/* -------------------------------------------------------------------------- */

function getLineLetters(context, wordsArray, textAlign, contentOffsetLeft, fitWidth, fontSize){
	var widthOf = context.measureText,
		textLine = wordsArray.join(' '),
		
		nb_words = wordsArray.length,
		nb_spaces = nb_words-1,
		nb_letters = textLine.length - nb_spaces,

		spacewidth = 0,
		spacing = (textAlign == "justify") ? fontSize/3 : fontSize/10,

		linegap = 0,
		gap = 0,
		offgap = 0,

		offset = 0,
		__cache = [],

		position = 0,
		letters = [];

	// cache width of letters
	var cachedLetterWidth = function(char){
		return __cache[char] ? __cache[char] : __cache[char] = widthOf(char);
	}

	context.setFontSize(fontSize);
	spacewidth = widthOf(" ");
	linegap = fitWidth - widthOf(textLine);

	switch(textAlign) {
		case "justify" :
			gap = (linegap/(textLine.length-1));
			break;
		case "left" :
			break;
		case "right" :
			offset = linegap;
			break;
		case "center" :
			offset = linegap/2;
			break;
		default :
			break;
	}

	for (var i=0; i<textLine.length; i++){
		var char = textLine[i],
			letterWidth = cachedLetterWidth(char);

		if (textAlign=="justify"){
			if (char == " "){
				letterWidth += spacing;
			} else {
				letterWidth -= spacing*nb_spaces/nb_letters;
			}
		}
		/*
		if ( char.charCodeAt(0) == 9) {
			letterWidth = 6 * spacewidth;
		};
		*/

		letters[i] = {
			char : char,
			position : contentOffsetLeft + offset + position + offgap,
			width : letterWidth,
			linegap : linegap,
			selected : false
		};
		position += letterWidth;
		offgap += gap;
	}

	// last letter Position Approximation Corrector
	
	if (letters[textLine.length-1]) {
		var last = letters[textLine.length-1],
			delta = fitWidth - (last.position + last.width);

		if ((0.05 + last.position + last.width) > fitWidth+contentOffsetLeft) {
			last.position = Math.floor(last.position - delta - 0.5) - contentOffsetLeft;
		}

		letters[i] = {
			char : " ",
			position : contentOffsetLeft + offset + position + offgap,
			width : 10,
			linegap : linegap,
			selected : false
		};
	}
	return letters;
}

function getTextMatrixLines(element){
	var	paragraphe = element.label.split(/\r\n|\r|\n/),

		lineHeight = element.lineHeight,
		fitWidth = element.width,
		textAlign = element.textAlign,
		fontSize = element.fontSize,
		fontType = element.fontType,
		contentOffsetLeft = element.contentOffsetLeft,
		contentOffsetRight = element.contentOffsetRight,

		matrix = [],
		wordsArray = [],

		k = 0,
		currentLine = 0,
		context = element.layer.context;


	context.setFontSize(fontSize);
	context.fontType = fontType;

	for (var i = 0; i < paragraphe.length; i++) {
		var words = paragraphe[i].split(' '),
			idx = 1;

		while (words.length>0 && idx <= words.length) {
			var str = words.slice(0, idx).join(' '),
				w = context.measureText(str);

			var offLeft = contentOffsetLeft[k] ? contentOffsetLeft[k] : 0,
				offRght = contentOffsetRight[k] ? contentOffsetRight[k] : 0,

				currentFitWidth = fitWidth - offRght - offLeft;

			if (w > currentFitWidth) {
				idx = (idx == 1) ? 2 : idx;

				wordsArray = words.slice(0, idx - 1);

				matrix[currentLine++] = {
					text : wordsArray.join(' '),
					align : textAlign,
					words : wordsArray,
					letters : getLineLetters(
								context,
								wordsArray, textAlign, 
								offLeft,
								currentFitWidth,
								fontSize
							  )
				};

				k++;
				
				words = words.splice(idx - 1);
				idx = 1;

			} else {
				idx++;
			}

		}

		// last line
		if (idx > 0) {

			var align = (textAlign=="justify") ? "left" : textAlign,

				offLeft = contentOffsetLeft[currentLine] ?
									contentOffsetLeft[currentLine] : 0,

				offRght = contentOffsetRight[currentLine] ?
									contentOffsetRight[currentLine] : 0,

				currentFitWidth = fitWidth - offRght - offLeft;

			matrix[currentLine] = {
				text : words.join(' '),
				align : align,
				words : words,
				letters : getLineLetters(
							context,
							words, align, 
							offLeft,
							currentFitWidth,
							fontSize
						  )
			};

		}

		currentLine++;
	}

	return matrix;

};

function printTextMatrix(context, textMatrix, caret, x, y, vOffset, viewportWidth, viewportHeight, viewportTop, lineHeight, fontSize, fontType, color, caretOpacity){
	context.setFontSize(fontSize);
	context.fontType = fontType;
	var letters = [];

	var start = -Math.ceil((y - viewportTop)/lineHeight),
		
		end = Math.min(
			textMatrix.length, 
			start + Math.ceil(viewportHeight/lineHeight)
		);

	if (start-1 >= 0) start--;
	if (end+1 <= textMatrix.length) end++;

	for (var line=start; line<end; line++){
		var tx = x,
			ty = y + vOffset + lineHeight * line;

		// only draw visible lines
		if ( ty < (viewportTop + viewportHeight + lineHeight) && ty >= viewportTop) {
			letters = textMatrix[line].letters;

			context.setColor("rgba(180, 180, 255, 0.60)");
			context.highlightLetters(letters, tx, ty - vOffset, lineHeight);
	
			context.setColor(color);
			context.drawLetters(letters, tx, ty);

		}
	}
}

Canvas.implement({
	highlightLetters : function(letters, x, y, lineHeight){
		var c, nush, cx, cy, cw;
		for (var i=0; i<letters.length; i++){
			c = letters[i];
	 		nush = letters[i+1] ? 
	 						letters[i+1].position - c.position - c.width : 0;

		 	cx = x + c.position;
 			cy = y;
 			cw = c.width + nush + 0.25;
 			if (c.selected){
				this.fillRect(cx, cy, cw, lineHeight);
			}
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




/* -------------------------- */
/* Native (@) 2012 Stight.com */
/* -------------------------- */

UIElement.extend("UIText", {
	init : function(){
		var self = this;

		this._area = null;
		this.flags._canReceiveFocus = true;

		this.caret = {
			x1 : 8,
			y1 : 0,
			x2 : 40,
			y2 : 6
		};

		if (!this.color){
			this.color = "#000000";
		}

		this.addEventListener("mouseover", function(e){
			this.verticalScrollBar.fadeIn(150, function(){
				/* dummy */
			});
		}, false);

		this.addEventListener("mouseout", function(e){
			this.verticalScrollBar.fadeOut(400, function(){
				/* dummy */
			});
		}, false);

		this.addEventListener("dragstart", function(e){
			this.__startTextSelectionProcessing = true;
			this.__startx = e.x;
			this.__starty = e.y + this.scroll.top;
		}, false);

		layout.rootElement.addEventListener("dragover", function(e){
			if (self.__startTextSelectionProcessing) {
				self.area = {
					x1 : self.__startx,
					y1 : self.__starty,
					x2 : e.x,
					y2 : e.y + self.scroll.top
				};
			}
		}, false);

		this.addEventListener("dragend", function(e){
			this.__startTextSelectionProcessing = false;
		}, false);

		this.addEventListener("mousedown", function(e){
			if (this.area) {
				this.area = null;
			}
			if (this.scroll.scrolling){
				Timers.remove(this.scroll.timer);
				this.scroll.scrolling = false;
				this.scroll.initied = false;
			}
		}, false);

		this.addEventListener("mousewheel", function(e){
			if (this.h / this.scrollBarHeight < 1) {
				canvas.__mustBeDrawn = true;
				this.scrollY(1 + (-e.yrel-1) * 18);
			}
		}, false);

		this.addEventListener("focus", function(e){
		}, false);


		/* --------------------- */


		this._textMatrix = getTextMatrixLines(this.text, this.lineHeight, this.w, this.textAlign, this.fontSize);
		this.content.height = this.lineHeight * this._textMatrix.length;


		/* --------------------- */


		this.verticalScrollBar = this.add("UIVerticalScrollBar");
		this.verticalScrollBarHandle = this.verticalScrollBar.add("UIVerticalScrollBarHandle");

	},

	draw : function(){
		var params = {
				x : this._x,
				y : this._y,
				w : this.w,
				h : this.h
			},

			pad = {
				left : 0,
				right : 0,
				top : 0,
				bottom : 0
			},

			x = params.x + pad.left,
			y = params.y + pad.top,
			w = params.w - pad.right - pad.left,
			h = params.h - pad.top - pad.bottom,

			vOffset = (this.lineHeight/2)+5;



		//if (this.__cache) {
			//canvas.putImageData(this.__cache, params.x, params.y);
		//} else {

			canvas.save();
				if (this.background){
					canvas.fillStyle = this.background;
				}
				canvas.roundbox(params.x, params.y, params.w, params.h, this.radius, this.background, false); // main view
				canvas.clip();
		
				if (this.area) {
					this.caret = setTextSelection(this._textMatrix, x, y, this.scroll.top, this.lineHeight, w, this.area, this.__startx, this.__starty);
					this.selection = drawCaretSelection(this._textMatrix, this.caret, x, y - this.scroll.top, this.lineHeight);
				}

				canvas.fillStyle = this.color;
				canvas.fontSize = this.fontSize;
				canvas.fontType = this.fontType;

				printTextMatrix(this._textMatrix, x, y + vOffset - this.scroll.top, w, h, params.y, this.lineHeight);

			canvas.restore();
			//this.__cache = canvas.getImageData(params.x, params.y, params.w, params.h);
		//}

	}
});

function drawCaretSelection(textMatix, caret, x, y, lineHeight){
	var m = textMatix,
		cx = x,
		cy = y,
		cw = 2,
		size = 0,
		selection = [],

		c = caret;

	canvas.fillStyle = "rgba(180, 180, 255, 0.60)";

	c.y1 = c.y1.bound(0, m.length-1);
	c.y2 = c.y2.bound(0, m.length-1);

	if (c.y2 <= c.y1) {
		let x1 = Math.min(c.x1, c.x2),
			y1 = Math.min(c.y1, c.y2),
			x2 = Math.max(c.x1, c.x2),
			y2 = Math.max(c.y1, c.y2);

		c = {
			x1 : x1,
			y1 : y1,
			x2 : x2,
			y2 : y2
		};
	}

	c.x1 = c.x1.bound(0, m[c.y1].letters.length - 1);
	c.x2 = c.x2.bound(0, m[c.y2].letters.length - 1);

 	var line = c.y1,
 		char = c.x1,
 		doit = true;

 	while (doit) {
		if (line >= c.y2 && char >= c.x2) {
			doit = false;
		}

 		let letter = m[line].letters[char],
 			nush = m[line].letters[char+1] ? m[line].letters[char+1].position - letter.position - letter.width : 0;

 		cx = x + letter.position;
 		cy = y + line * lineHeight + 1;
 		cw = letter.width + nush + 0.25;
 		selection.push(letter.char);

		canvas.fillRect(cx, cy, cw, lineHeight);

 		size++;
 		char++;

 		if (char > m[line].letters.length - 1) {
 			char = 0;
 			line++;
 		}
		

 	};

 	return {
 		size : size,
 		text : selection.join('')
 	};

}

function getCaretSelection(textMatrix, r, lineHeight){
	var x1 = r.x1, //Math.min(r.x1, r.x2),
		x2 = r.x2, //Math.max(r.x1, r.x2),
		c = {
			y1 : r.y1/lineHeight,
			y2 : r.y2/lineHeight - 1
		};

	var getCharPosition = function(letters, x, flag){
		for (var i=0; i<letters.length; i++){
			let o = flag ? letters[i].width/2 : 0;
			if (x+o < letters[i].position) {
				break;
			}
		}
		return Math.max(0, i-1);
	};

	c.y1 = c.y1.bound(0, textMatrix.length-1);
	c.y2 = c.y2.bound(0, textMatrix.length-1);

	var topLineLetters = textMatrix[c.y1].letters,
		bottomLineLetters = textMatrix[c.y2].letters;

	c.x1 = getCharPosition(topLineLetters, x1);
	c.x2 = getCharPosition(bottomLineLetters, x2);

	return c;
}

function setTextSelection(textMatrix, x, y, scrollTop, lineHeight, fitWidth, area, startPointX, startPointY){
	var context = canvas;

	y = y - scrollTop;

	if (area.y2 <= startPointY) {
		var x1 = area.x2,
			y1 = Math.min(area.y1, area.y2),
			x2 = area.x1,
			y2 = Math.max(area.y1, area.y2);

		area = {
			x1 : x1,
			y1 : y1,
			x2 : x2,
			y2 : y2
		};
	}

	var r = {
			x1 : (area.x1 - x),
			y1 : Math.floor((area.y1 - (y+scrollTop) ) / lineHeight) * lineHeight,
			x2 : (area.x2 - x),
			y2 : Math.ceil((area.y2 - (y+scrollTop)) / lineHeight) * lineHeight,
		};

	r.x1 = isNaN(r.x1) ? 0 : r.x1;
	r.y1 = isNaN(r.y1) ? 0 : r.y1;
	r.x2 = isNaN(r.x2) ? 0 : r.x2;
	r.y2 = isNaN(r.y2) ? 0 : r.y2;

	return getCaretSelection(textMatrix, r, lineHeight);

	/*
	var rw = r.x2 - r.x1,
		rh = r.y2 - r.y1;

		context.fillStyle = "rgba(255, 0, 0, 0.25)";
		context.fillRect(x+r.x1 , y+r.y1 , rw, rh);
	*/

	/*

	var	nb_lines = rh/lineHeight,
		ty = y;

	context.fillStyle = "rgba(180, 180, 255, 0.50)";

	if (nb_lines<=1) {
			context.fillRect(x+r.x1, ty + r.y1, rw, lineHeight);
	} else {

		context.fillRect(x+r.x1, ty + r.y1, fitWidth - r.x1, lineHeight);
		for (var i = 1; i < nb_lines-1; i++) {
			ty = y + i*lineHeight;
			context.fillRect(x , ty + r.y1, fitWidth, lineHeight);
		};
		ty = y + (nb_lines-1)*lineHeight;
		context.fillRect(x , ty + r.y1, r.x2, lineHeight);


	}
	*/

}

canvas.implement({
	drawLetterArray : function(letters, x, y){
		for (var i=0; i<letters.length; i++){
			let c = letters[i];
			this.fillText(c.char, x+c.position, y);
		}
	}
});

function getLineLetters(wordsArray, textAlign, fitWidth, fontSize){
	var context = new Canvas(640, 480),
		widthOf = context.measureText,
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

	context.fontSize = fontSize;
	spacewidth = widthOf(" ");
	linegap = fitWidth - widthOf(textLine);

	switch(textAlign) {
		case "justify" :
			gap = (linegap/(textLine.length-1));
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
		let char = textLine[i],
			letterWidth = cachedLetterWidth(char);

		if (textAlign=="justify"){
			if (char==" "){
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
			position : offset + position + offgap,
			width : letterWidth,
			linegap : linegap
		};
		position += letterWidth;
		offgap += gap;
	}

	// last letter Position Approximation Corrector
	var last = letters[textLine.length-1],
		delta = fitWidth - (last.position + last.width);

	if ((0.05 + last.position + last.width) > fitWidth) {
		last.position = Math.floor(last.position - delta - 0.5);
	}

	return letters;
}

function getTextMatrixLines(text, lineHeight, fitWidth, textAlign, fontSize){
	var	paragraphe = text.split(/\r\n|\r|\n/),
		wordsArray = [],
		currentLine = 0,
		context = new Canvas(1, 1);

	var matrix = [];

	context.fontSize = fontSize;

	for (var i = 0; i < paragraphe.length; i++) {
		var words = paragraphe[i].split(' '),
			idx = 1;

		while (words.length > 0 && idx <= words.length) {
			var str = words.slice(0, idx).join(' '),
				w = context.measureText(str);

			if (w > fitWidth) {
				idx = (idx == 1) ? 2 : idx;

				wordsArray = words.slice(0, idx - 1);

				matrix[currentLine++] = {
					text : wordsArray.join(' '),
					align : textAlign,
					words : wordsArray,
					letters : getLineLetters(wordsArray, textAlign, fitWidth, fontSize)
				};
				
				words = words.splice(idx - 1);
				idx = 1;

			} else {
				idx++;
			}

		}

		// last line
		if (idx > 0) {

			let align = (textAlign=="justify") ? "left" : textAlign;
			matrix[currentLine] = {
				text : words.join(' '),
				align : align,
				words : words,
				letters : getLineLetters(words, align, fitWidth, fontSize)
			};

		}

		currentLine++;
	}

	//UIView.content.height = currentLine*lineHeight;

	return matrix;

};

function printTextMatrix(textMatrix, x, y, viewportWidth, viewportHeight, viewportTop, lineHeight){
	for (var line=0; line<textMatrix.length; line++){
		var tx = x,
			ty = y + lineHeight * line;

		// only draw visible lines
		if ( ty < (viewportTop + viewportHeight + lineHeight) && ty >= viewportTop) {
			canvas.drawLetterArray(textMatrix[line].letters, tx, ty);
		}
	}
}



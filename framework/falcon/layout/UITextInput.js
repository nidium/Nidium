/* -------------------------- */
/* Native (@) 2012 Stight.com */
/* -------------------------- */

Native.elements.export("UITextInput", {
	init : function(){
		var self = this,
			scrollY = this.scrollTop;

		this.canReceiveFocus = true;
		this.caretOpacity = 1;
		this.caretCounter = 0;

		this.background = OptionalValue(this.options.background, '#ffffff');
		this.color = OptionalValue(this.options.color, "#000000");

		this.offsetLeft = OptionalValue(this.options.offsetLeft, []);
		this.offsetRight = OptionalValue(this.options.offsetRight, []);

		this.padding = {
			left : 0,
			right : 0,
			top : 0,
			bottom : 0
		};

		/* ------------------------------------------------------------------ */

		this.setText = function(text){
			this.text = text;

			this._textMatrix = getTextMatrixLines(this);
			this.mouseSelectionArea = null;

			this.selection = {
				text : '',
				offset : 0,
				size : 0
			};

			this.caret = {
				x1 : 0,
				y1 : 0,
				x2 : 0,
				y2 : 0
			};

		};

		this.setLayout = function(offsetLeft, offsetRight){
			this.offsetLeft = OptionalValue(offsetLeft, []);
			this.offsetRight = OptionalValue(offsetRight, []);
			this.setText(this.text);
		};

		/* ------------------------------------------------------------------ */

		this.addEventListener("mousedown", function(e){
			if (this.mouseSelectionArea) {
				this.mouseSelectionArea = null;
			}

			this._startMouseSelection(e);
			this._doMouseSelection(e);
			this._endMouseSelection();
			this.setCaret(this.selection.offset, 0);
			this.setStartPoint();
			this.caretCounter = -20;

			this.select(false);
		}, false);

		/* ------------------------------------------------------------------ */

		this.addEventListener("dragstart", function(e){
			self._startMouseSelection(e);
		}, false);

		document.addEventListener("dragover", function(e){
			if (!self.__startTextSelectionProcessing) return false;

			self._doMouseSelection(e);

		}, false);

		this.addEventListener("dragend", function(e){
			self._endMouseSelection();
			self.fireEvent("textselect", self.selection);
		}, false);

		/* ------------------------------------------------------------------ */

		this.addEventListener("keydown", function(e){
			if (!this.hasFocus) return false;

			if (e.ctrlKey) {
				switch (e.keyCode) {
					case 88 : // CTRL-X
						this.cut();
						break;

					case 67 : // CTRL-C
						this.copy();
						break;

					case 86 : // CTRL-V
						this.paste();
						break;
				}
			} else {
				switch (e.keyCode) {
					case 8 : // backspace
						if (this.selection.offset-1 < 0) return false;
						
						this._insert(
							'', 
							this.selection.offset, 
							this.selection.size, 
							this.selection.offset-1, 
							0
						);

						this._insert(
							'', 
							this.selection.offset, 
							1, 
							this.selection.offset, 
							0
						);

						break;

					case 127 : // delete
						this._insert(
							'', 
							this.selection.offset, 
							Math.max(1, this.selection.size), 
							this.selection.offset, 
							0
						);
						break;

					case 1073741906 : // up
						if (this.caret.y1 >=1 && this.caret.y2 >= 1){
							if (e.shiftKey){
								this.caret.y1--;
							} else {
								this.resetStartPoint();
								this.caret.y1--;
								this.caret.y2--;
							}
							this.setSelectionFromMatricialCaret(this.caret);
						}
						break;

					case 1073741905 : // down
						var nblines = this._textMatrix.length-1;
						if (this.caret.y1 < nblines && this.caret.y2 < nblines){
							if (e.shiftKey){
								this.caret.y1++;
							} else {
								this.resetStartPoint();
								this.caret.y1++;
								this.caret.y2++;
							}
							this.setSelectionFromMatricialCaret(this.caret);
						} 
						break;

					case 1073741903 : // left
						if (e.shiftKey){
							this.setCaret(
								++this.selection.offset, 
								--this.selection.size
							);
						} else {
							this.resetStartPoint();
							this.setCaret(
								++this.selection.offset, 
								0
							);
						}
						break;

					case 1073741904 : // right
						if (e.shiftKey){
							this.setCaret(
								--this.selection.offset, 
								++this.selection.size
							);
						} else {
							this.resetStartPoint();
							this.setCaret(
								--this.selection.offset, 
								0
							);
						}
						break;

					default : break;
				}
			}
		}, false);

		this.addEventListener("textinput", function(e){
			if (!this.hasFocus) return false;
			this.insert(e.text);
		}, false);

		this.addEventListener("focus", function(e){
		}, false);

		/* ------------------------------------------------------------------ */

		this.setStartPoint = function(){
			this._StartCaret = {
				x : this.caret.x1,
				y : this.caret.y1
			};
			//console.log(this._StartCaret);
		};

		this.resetStartPoint = function(){
			delete(this._StartCaret);
		};


		this._startMouseSelection = function(e){
			this.__startTextSelectionProcessing = true;
			this.__startx = e.x;
			this.__starty = e.y + (this.parent ? this.parent.scrollTop : 0);
		};

		this._endMouseSelection = function(e){
			this.__startTextSelectionProcessing = false;
		};

		this._doMouseSelection = function(e){
			if (self.__startTextSelectionProcessing) {
				self.mouseSelectionArea = {
					x1 : self.__startx,
					y1 : self.__starty,
					x2 : e.x,
					y2 : e.y + this.parent.scrollTop
				};

				var area = self.mouseSelectionArea;

				if (area.y2 <= self.__starty) {
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

				var x = self.__left + self.padding.left,
					y = self.__top + self.padding.top - scrollY,

					r = {
						x1 : (area.x1 - x),
						y1 : Math.floor((area.y1 - y) / self.lineHeight) * self.lineHeight,
						x2 : (area.x2 - x),
						y2 : Math.ceil((area.y2 - y) / self.lineHeight) * self.lineHeight
					};

				r.x1 = isNaN(r.x1) ? 0 : r.x1;
				r.y1 = isNaN(r.y1) ? 0 : r.y1;
				r.x2 = isNaN(r.x2) ? 0 : r.x2;
				r.y2 = isNaN(r.y2) ? 0 : r.y2;

				self.setMatricialCaretFromRelativeMouseSelection(r);
				self.setSelectionFromMatricialCaret(self.caret);

			}
		};

		/* ------------------------------------------------------------------ */

		this.setMatricialCaretFromRelativeMouseSelection = function(r){
			var m = this._textMatrix;
				x1 = r.x1,
				x2 = r.x2,
				c = {
					y1 : r.y1/this.lineHeight,
					y2 : r.y2/this.lineHeight - 1
				};

			var getCharPosition = function(line, x){
				var i = 0, l = line.letters;
				while (i<l.length && x > l[i].position + l[i].width/2){	i++; }
				return i;
			};

			c.y1 = c.y1.bound(0, m.length-1);
			c.y2 = c.y2.bound(0, m.length-1);
			c.x1 = getCharPosition(m[c.y1], x1); // letters of line y1
			c.x2 = getCharPosition(m[c.y2], x2); // letters of line y2

		 	if (c.x1 < 0){
		 		c.x1 = 0;
		 	}

			if (c.y2 <= c.y1) {
				var x1 = Math.min(c.x1, c.x2),
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

			this.caret = c;
		};

		/* ------------------------------------------------------------------ */

		this.select = function(state){
			/* 2 times faster than the old while loop method */
			var m = this._textMatrix,
				chars, x = y = 0,
				offset = this.selection.offset,
				state = (typeof(state) == "undefined") ? 
										true : state ? true : false;
			
			for (y=0; y<m.length; y++)
				for (x=0, chars = m[y].letters; x<chars.length; x++){
					chars[x].selected = state;
				}

			if (state) {
				var lastLine = this._textMatrix.length - 1;
				this.selection = {
					text : this.text,
					offset : 0,
					size : this.text.length-1
				};
				this.caret = {
					x1 : 0,
					y1 : 0,
					x2 : this._textMatrix[lastLine].letters.length - 1,
					y2 : lastLine
				}
			} else {
				this.setCaret(offset, 0);
			}
		};

		this._insert = function(text, offset, size, newoffset, newsize){
			this.setText(this.text.splice(offset, size, text));
			this.setCaret(newoffset, newsize);
		};

		this.replace = function(text){
			this._insert(
				text, 
				this.selection.offset, 
				this.selection.size, 
				this.selection.offset + text.length,
				0
			);
		};

		this.insert = function(text){
			this._insert(
				text, 
				this.selection.offset, 
				this.selection.size, 
				++this.selection.offset, 
				0
			);
		};

		this.append = function(text){
			this.setText(this.text + text);
		};

		this.cut = function(){
			this.copy();
			this._insert(
				'', 
				this.selection.offset, 
				this.selection.size, 
				this.selection.offset, 
				0
			);
		};

		this.copy = function(){
			if (this.selection.size>0) {
				Native.pasteBuffer = this.selection.text;
			} else {
				Native.pasteBuffer = '';
			}
		};

		this.paste = function(){
			if (Native.pasteBuffer != '') {
				this.replace(Native.pasteBuffer);
			}
		};

		/* ------------------------------------------------------------------ */

		this.getTextSelection = function(){
			return this.selection.text;
		};

		/* ------------------------------------------------------------------ */

		this.getCaret = function(){
			return this.selection;
		};

		this.setCaret = function(offset, size){
			/* 3 times faster than setSelectionFromMatricialCaret */
			offset = Math.max(
				0,
				(typeof(offset)!=undefined ? parseInt(offset, 10) : 0)
			);

			size = Math.max(
				0,
				(typeof(size)!=undefined ? parseInt(size, 10) : 0)
			);

			var m = this._textMatrix,
				chars, o = offset, s = size,
				walk = x = y = 0, 
				select = false,
				__setted__ = false,
				c = {
					x1 : false,
					y1 : false,
					x2 : false,
					y2 : false
				};

			for (y=0; y<m.length; y++)
				for (x=0, chars = m[y].letters; x<chars.length && s>-1; x++){
					chars[x].selected = false;

					if (walk >= o) {
						select = true;
					}

					if (select){
						if (!__setted__) {
							c.x1 = x;
							c.y1 = y;
							__setted__ = true;
						}
						if (s>=1) {
							chars[x].selected = true;
						}
				 		s--;
					}

					if (s <= 0) {
						c.x2 = x;
						c.y2 = y;
						select = false;
					}

					walk++;

				}

			this.caret = c;

			this.selection = {
		 		text : this.text.substr(offset, size),
		 		offset : offset,
		 		size : size
		 	}

			this.caretCounter = 0;

		 	return this.selection;
		};

		this.setSelectionFromMatricialCaret = function(c){
			/* 10 times faster than the old method */
			var m = this._textMatrix,
				selection = [],
				chars,
				walk = x = y = 0,

				line = c.y1,
		 		char = c.x1,
		 		select = false,

				size = 0,
				offset = 0,
				selection = [];

		 	var selectLetter = function(line, char){
		 		var letter = m[line].letters[char],
		 			nush = m[line].letters[char+1] ? m[line].letters[char+1].position - letter.position - letter.width : 0;

		 		letter.selected = true;
		 		letter.fullWidth = letter.width + nush + 0.25
		 		selection.push(letter.char);
		 		size++;
		 	};

		 	c.x1 = Math.min(c.x1, m[c.y1].letters.length - 1);
		 	c.x2 = Math.min(c.x2, m[c.y2].letters.length - 1);

			for (y=0; y<m.length; y++) for (x=0, chars = m[y].letters; x<chars.length; x++){
				chars[x].selected = false;

				if (x == c.x1 && y == c.y1) {
					offset = walk;
					select = true;
				}

				if (x == c.x2 && y == c.y2) {
					select = false;
				}

				if (select){
					chars[x].selected = true;
			 		size++;
				}

				walk++;
			}

		 	this.selection = {
		 		text : this.text.substr(offset, size),
		 		offset : offset,
		 		size : size
		 	};

			this.caretCounter = 0;
			this.redraw();
		};

		/* ------------------------------------------------------------------ */
		

		blinkingCaret = false;

		this.caretTimer = setInterval(function(){
			if (self.selection.size == 0 && self.hasFocus) {
				self.caretCounter++;

				if (self.caretCounter<=20){
					self.caretOpacity = 1;					
				} else if (self.caretCounter>20 && self.caretCounter<60) {
					self.caretOpacity *= 0.85;
				}

				if (self.caretCounter>=60){
					self.caretCounter = 0;
				}
			} else {
				self.caretOpacity = 0;
			}
			self.redraw();

		}, 16);

		this.setText(this.text);

	},

	/* ---------------------------------------------------------------------- */
	/* - DRAW --------------------------------------------------------------- */
	/* ---------------------------------------------------------------------- */

	draw : function(context){
		var params = this.getDrawingBounds(),

			x = params.x + this.padding.left,
			y = params.y + this.padding.top,
			w = params.w - this.padding.right - this.padding.left,
			h = params.h - this.padding.top - this.padding.bottom,

			vOffset = (this.lineHeight/2)+5;

		printTextMatrix(
			context,
			this._textMatrix,
			this.caret,
			x,
			y - (this.parent ? this.parent.scrollTop : 0), 
			vOffset, 
			w,
			h, 
			params.y, 
			this.lineHeight,
			this.fontSize,
			this.fontType,
			this.color, 
			this.caretOpacity
		);


	}
});

function getLineLetters(context, wordsArray, textAlign, offsetLeft, fitWidth, fontSize){
	var widthOf = context.measureText,
		textLine = wordsArray.join(' '),
		
		nb_words = wordsArray.length,
		nb_spaces = nb_words-1,
		nb_letters = textLine.length - nb_spaces,

		spacewidth = 0,
		spacing = (textAlign == "justify") ? fontSize/10 : fontSize/10,

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
			position : offsetLeft + offset + position + offgap,
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

		if ((0.05 + last.position + last.width) > fitWidth+offsetLeft) {
			last.position = Math.floor(last.position - delta - 0.5) - offsetLeft;
		}

		letters[i] = {
			char : " ",
			position : offsetLeft + offset + position + offgap,
			width : 10,
			linegap : linegap,
			selected : false
		};
	}
	return letters;
}

function getTextMatrixLines(element){
	var	paragraphe = element.text.split(/\r\n|\r|\n/),

		lineHeight = element.lineHeight,
		fitWidth = element.width,
		textAlign = element.textAlign,
		fontSize = element.fontSize,
		offsetLeft = element.offsetLeft,
		offsetRight = element.offsetRight,

		matrix = [],
		wordsArray = [],

		k = 0,
		currentLine = 0,
		context = element.layer.context;


	context.setFontSize(fontSize);

	for (var i = 0; i < paragraphe.length; i++) {
		var words = paragraphe[i].split(' '),
			idx = 1;

		while (words.length > 0 && idx <= words.length) {
			var str = words.slice(0, idx).join(' '),
				w = context.measureText(str);

			var offLeft = offsetLeft[k] ? offsetLeft[k] : 0,
				offRght = offsetRight[k] ? offsetRight[k] : 0,

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

				offLeft = offsetLeft[currentLine] ?
									offsetLeft[currentLine] : 0,

				offRght = offsetRight[currentLine] ?
									offsetRight[currentLine] : 0,

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
	var letters = [];
	var start = -Math.ceil((y - viewportTop)/lineHeight),
		
		end = Math.min(
			textMatrix.length, 
			start + Math.ceil(viewportHeight/lineHeight)
		);


	context.setFontSize(fontSize);
	context.fontType = fontType;

	if (start-1 >= 0) start--;
	if (end+1 <= textMatrix.length) end++;

	for (var line=start; line<end; line++){
		var tx = x,
			ty = y + vOffset + lineHeight * line;

		// only draw visible lines
		//if ( ty < (viewportTop + viewportHeight + lineHeight) && ty >= viewportTop) {
			letters = textMatrix[line].letters;

			context.setColor("rgba(180, 180, 255, 0.60)");
			context.highlightLetters(letters, tx, ty - vOffset, lineHeight);
	
			context.setColor(color);
			if (line == caret.y2 && caretOpacity >= 0.10) {
				context.drawLettersWithCaret(
					letters, 
					tx, ty, 
					lineHeight, vOffset, caret.x2, caretOpacity
				);
			} else {
				context.drawLetters(letters, tx, ty);
			}

		//}
	}
}




/* -------------------------- */
/* Native (@) 2012 Stight.com */
/* -------------------------- */

Native.elements.export("UITextInput", {
	public : {
		label : {
			set : function(value){
				if (this.multiline === false) this.resizeElement();
			}
		},

		fontSize : {
			set : function(value){
				if (this.multiline === false) this.resizeElement();
			}
		},

		fontType : {
			set : function(value){
				if (this.multiline === false) this.resizeElement();
			}
		}
	},

	init : function(){
		var o = this.options,
			self = this;

		this.caretOpacity = 1;
		this.caretCounter = 0;

		this.setProperties({
			canReceiveFocus	: true,
			fontSize  		: OptionalNumber(o.fontSize, 11),
			fontType  		: OptionalString(o.fontType, "arial"),

			offsetLeft 		: OptionalValue(o.offsetLeft, []),
			offsetRight		: OptionalValue(o.offsetRight, []),

			multiline 		: OptionalBoolean(o.multiline, true),

			background 		: OptionalValue(o.background, '#ffffff'),
			color 			: OptionalValue(o.color, "#000000")
		});

		this.resizeElement = function(){
			var line = this._textMatrix ? this._textMatrix[0] : false,
				letters = line ? line.letters : false,
				nb_letters = letters ? letters.length : 0,
				width = nb_letters ? letters[nb_letters-1].position + letters[nb_letters-1].width*2 : 0;

			this._innerTextWidth = width;

			var minWidth = Math.max(this._innerTextWidth, this.parent.width);

			this.width = Math.min(minWidth, 16364);
			this.overlay.width = Math.min(minWidth, 16364);
		};

		this.padding = {
			left : 0,
			right : 0,
			top : 0,
			bottom : 0
		};

		this.overlay = new UIElement(this, {
			width : this.parent.width,
			heigh : this.height,
		});

		/* ------------------------------------------------------------------ */

		this.setText = function(text){
			this.text = text;

			if (this.multiline === false) {
				/* TODO : Fix Layer Issue (width is limited to 16384) */
				/* so we limit nb chars to 2048 for inline input field */
				this.text = text.substr(0, 2048);
				this._textMatrix = getTextLine(this);
				this.resizeElement();
			} else {
				this._textMatrix = getTextMatrixLines(this);
			}

			this.mouseSelectionArea = null;
			
			this.height = Math.max(
				this._textMatrix.length * this.lineHeight,
				this.parent.height
			);
			this.overlay.height = this._height;

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

			this.redraw();
		};

		this.setLayout = function(offsetLeft, offsetRight){
			this.offsetLeft = OptionalValue(offsetLeft, []);
			this.offsetRight = OptionalValue(offsetRight, []);
			this.setText(this.text);
		};

		/* ------------------------------------------------------------------ */

		this.overlay.addEventListener("mousedown", function(e){
			if (self.mouseSelectionArea) {
				self.mouseSelectionArea = null;
			}

			self._startMouseSelection(e);
			self._doMouseSelection(e);
			self._endMouseSelection();

			self.setCaret(self.selection.offset, 0);
			self.setStartPoint();
			self.caretCounter = -20;

			self.select(false);
		}, false);

		/* ------------------------------------------------------------------ */

		this.overlay.addEventListener("mousedblclick", function(e){
			self.resetStartPoint();
			self.select();
		}, false);

		this.overlay.addEventListener("dragstart", function(e){
			self._startMouseSelection(e);
			self.startDragTimer();
		}, false);

		document.addEventListener("dragover", function(e){
			if (!self.__startTextSelectionProcessing) return false;
			self._doMouseSelection(e);

			var minx = self.parent.__left,
				maxx = minx + self.parent._width;

			if (self._autoScrollLeft && self.isPointInside(e.x, e.y)) {
				self._autoScrollLeft = false;
			} else {
				self._autoScrollLeft = false;

				if (e.x > maxx){
					self._autoScrollLeft = -(e.x - maxx)/2;
				}
				if (e.x < minx){
					self._autoScrollLeft = (minx - e.x)/2;
				}
			}

		}, false);

		this.overlay.addEventListener("dragend", function(e){
			self._endMouseSelection();
			self._autoScrollLeft = false;
			self.stopDragTimer();
			self.fireEvent("textselect", self.selection);
		}, false);

		/* ------------------------------------------------------------------ */

		this.startDragTimer = function(){
			clearInterval(this.dragTimer);
			this.dragTimerPixelCount = 0;
			this.dragTimer = setInterval(function(){
				var m = self.text.length;
				if (self._autoScrollLeft) {
					if (self.parent.scrollLeft < 0) {
						self.stopDragTimer();
						self.parent.scrollLeft = 0;
						return;
					}

					self.parent.scrollLeft -= self._autoScrollLeft;
					self._doMouseSelection({
						x : window.mouseX,
						y : window.mouseY
					});
				}
			}, 16);
		};

		this.stopDragTimer = function(){
			clearInterval(this.dragTimer);
		};

		/* ------------------------------------------------------------------ */

		this.overlay.addEventListener("keydown", function(e){
			if (!self.hasFocus) return false;

			if (e.ctrlKey || e.cmdKey) {
				switch (e.keyCode) {
					case 65 : // CTRL-A
						self.select();
						break;

					case 88 : // CTRL-X
						self.cut();
						break;

					case 67 : // CTRL-C
						self.copy();
						break;

					case 86 : // CTRL-V
						self.paste();
						self.setStartPoint();
						break;

					case 70 : // CTRL-F
						//self.scrollToCaretCenter(self.caret.x1, self.caret.y1);
						self.scrollToSelectionEnd();
						break;
				}
			} else {
				switch (e.keyCode) {
					case 8 : // backspace
						if (self.multiline && self.selection.offset-1 < 0) return false;
						self._insert(
							'', 
							self.selection.offset, 
							self.selection.size, 
							self.selection.offset-1, 
							0
						);

						self._insert(
							'', 
							self.selection.offset, 
							1, 
							self.selection.offset, 
							0
						);
						break;

					case 13 : // enter
						break;

					case 127 : // delete
						self._insert(
							'', 
							self.selection.offset, 
							Math.max(1, self.selection.size), 
							self.selection.offset, 
							0
						);
						break;

					case 1073741898 : // LineStart
						if (this.multiline) return false;
						self.selection.text = "";
						self.selection.offset = 0;
						self.selection.size = 0;
						self.setCaret(0, 0);
						self.scrollToLineStart();
						break;

					case 1073741901 : // LineEnd
						var maxLength = self.text.length; // after last char
						if (this.multiline) return false;
						self.selection.text = "";
						self.selection.offset = maxLength;
						self.selection.size = 0;
						self.setCaret(maxLength, 0);
						self.scrollToLineEnd();
						break;

					case 1073741906 : // up
						if (this.multiline) return false;
						if (self.caret.y1 >= 1 && self.caret.y2 >= 1){
							if (e.shiftKey){
								self.caret.y1--;
							} else {
								self.resetStartPoint();
								self.caret.y1--;
								self.caret.y2--;
							}
							self.setSelectionFromMatricialCaret(self.caret);
						}
						break;

					case 1073741905 : // down
						if (this.multiline) return false;
						var nblines = self._textMatrix.length-1;
						if (self.caret.y1 < nblines && self.caret.y2 < nblines){
							if (e.shiftKey){
								self.caret.y1++;
							} else {
								self.resetStartPoint();
								self.caret.y1++;
								self.caret.y2++;
							}
							self.setSelectionFromMatricialCaret(self.caret);
						} 
						break;

					case 1073741903 : // right
						var maxLength = self.text.length-1,
							x1 = self.selection.offset,
							x2 = self.selection.offset + self.selection.size;

						if (e.shiftKey){
							if (x1 < self._StartCaret.x) {
								if (x1 <= maxLength) {
									self.selection.offset++;
									self.selection.size--;

									self.setCaret(
										self.selection.offset, 
										self.selection.size
									);
								}
							} else {
								if (x2 <= maxLength) {
									self.selection.size++;

									self.setCaret(
										self.selection.offset, 
										self.selection.size
									);

									self.scrollCheck(
										self.caret.x2,
										self.caret.y2	
									);
								}
							}
						} else {
							if (self.selection.size>0) {
								self.setCaret(
									self.selection.offset + self.selection.size, 
									0
								);

								self.setStartPoint();

								self.scrollCheck(
									self.caret.x2,
									self.caret.y2	
								);
							} else if (self.selection.offset <= maxLength) {
								self.selection.offset++;

								self.setCaret(
									self.selection.offset, 
									0
								);

								self.setStartPoint();

								self.scrollCheck(
									self.caret.x1,
									self.caret.y1	
								);
							}
						}
						break;

					case 1073741904 : // left
						var x1 = self.selection.offset,
							x2 = self.selection.offset + self.selection.size;

						if (e.shiftKey){
							if (x2 > self._StartCaret.x) {
								self.selection.size--;

								self.setCaret(
									self.selection.offset, 
									self.selection.size
								);
							} else {
								if (x1 > 0) {
									self.selection.offset--; 
									self.selection.size++;

									self.setCaret(
										self.selection.offset, 
										self.selection.size
									);

									self.scrollCheck(
										self.caret.x1,
										self.caret.y1	
									);
								}  
							}  
						} else {
							if (self.selection.size>0) {
								self.setCaret(
									self.selection.offset, 
									0
								);

								self.setStartPoint();

								self.scrollCheck(
									self.caret.x1,
									self.caret.y1	
								);
							} else if (self.selection.offset > 0) {
								self.selection.offset--;

								self.setCaret(
									self.selection.offset, 
									0
								);

								self.setStartPoint();

								self.scrollCheck(
									self.caret.x1,
									self.caret.y1	
								);
							}
						}
						break;

					default : break;
				}
			}
		}, false);

		this.overlay.addEventListener("textinput", function(e){
			if (!self.hasFocus) return false;
			self.insert(e.text);
			self.setStartPoint();
		}, false);

		this.addEventListener("focus", function(e){
			this.showCaret();
		}, false);

		this.addEventListener("blur", function(e){
			this.hideCaret();
		}, false);

		/* ------------------------------------------------------------------ */

		this.showCaret = function(){
			clearInterval(this.caretTimer);
			this.caretOpacity = 1;
			this.caretCounter = 0;
			this.updateOverlay();

			this.caretTimer = setInterval(function(){
				if (self.selection.size == 0 && self.hasFocus) {
					self.caretCounter++;

					if (self.caretCounter<=12){
						self.caretOpacity = 1;					
					} else if (self.caretCounter>12 && self.caretCounter<35) {
						self.caretOpacity *= 0.85;
					}

					if (self.caretCounter>=35){
						self.caretCounter = 0;
					}
				} else {
					self.caretOpacity = 0;
				}

				self.updateOverlay();

			}, 20);
		};

		this.hideCaret = function(){
			clearInterval(this.caretTimer);
			this.caretOpacity = 0;
			this.caretCounter = 0;
			this.updateOverlay();
		};

		this.setStartPoint = function(){
			this._StartCaret = {
				x : this.caret.x1,
				y : this.caret.y1
			};
		};

		this.resetStartPoint = function(){
			this._StartCaret = {
				x : 0,
				y : 0
			};
		};

		this._startMouseSelection = function(e){
			this.__startTextSelectionProcessing = true;
			this.__startx = e.x + self.parent.scrollLeft;
			this.__starty = e.y;
		};

		this._endMouseSelection = function(e){
			this.__startTextSelectionProcessing = false;
		};

		this._doMouseSelection = function(e){
			if (self.__startTextSelectionProcessing) {

				self.mouseSelectionArea = {
					x1 : self.__startx,
					y1 : self.__starty,
					x2 : e.x + self.parent.scrollLeft,
					y2 : e.y
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

				var x = self.__left + self.padding.left + self.parent.scrollLeft,
					y = self.__top + self.padding.top,
					lh = self.lineHeight,

					r = {
						x1 : (area.x1 - x),
						y1 : Math.floor((area.y1 - y) / lh) * lh,
						x2 : (area.x2 - x),
						y2 : Math.ceil((area.y2 - y) / lh) * lh
					};

				r.x1 = isNaN(r.x1) ? 0 : r.x1;
				r.y1 = isNaN(r.y1) ? 0 : r.y1;
				r.x2 = isNaN(r.x2) ? 0 : r.x2;
				r.y2 = isNaN(r.y2) ? 0 : r.y2;

				self.setMatricialCaretFromRelativeMouseSelection(r);
				if (self.multiline) {
					self.setSelectionFromMatricialCaret(self.caret);
				} else {
					self.setSelectionFromSingleLineCaret(self.caret);
				}
			}
		};

		/* ------------------------------------------------------------------ */

		this.scrollCheck = function(cx, cy){
			var x = this.matricialToPixel(cx, cy)[0],
				minx = this.parent.left,
				maxx = this.parent.left + this.parent.width - 20,

				tx = x + this.parent.left - this.parent.scrollLeft;

			if (tx > maxx) this.parent.updateScrollLeft(-this.parent.width/16);
			if (tx < minx) this.parent.updateScrollLeft(this.parent.width/16);
		};

		this.scrollToCaretCenter = function(cx, cy){
			var x = this.matricialToPixel(cx, cy)[0];
			this.parent.scrollLeft = x - this.parent.width/2;
		};

		this.scrollToSelectionStart = function(){
			var cx = this.caret.x1,
				cy = this.caret.y1,
				x = this.matricialToPixel(cx, cy)[0];
			this.parent.scrollLeft = x
		};

		this.scrollToSelectionEnd = function(){
			var cx = this.caret.x2,
				cy = this.caret.y2,
				x = this.matricialToPixel(cx, cy)[0];

			this.parent.scrollLeft = x - this.parent.width + 10;
		};


		this.scrollCheckSelection = function(x, y){
			var minx = this.parent.left + 10,
				maxx = this.parent.left + this.parent.width;

			if (x > maxx) {
				this.parent.layer.scrollLeft += 5;
			}
			if (x < minx) { this.parent.updateScrollLeft(1); }
		};

		this.scrollToLineStart = function(){
			var pos = this.parent.scrollLeft;
			this.parent.updateScrollLeft(pos);
		};

		this.scrollToLineEnd = function(){
			var pos = this.width;
			this.parent.updateScrollLeft(-pos);
		};

		this.matricialToPixel = function(cx, cy){
			var line = this._textMatrix[cy],
				letters = line ? line.letters : [],
				vOffset = (this.lineHeight/2) + 5,

				x = letters[cx] ? letters[cx].position : (
					letters[cx-1] ?
					letters[cx-1].position + letters[cx-1].width :
					0
				),

				y = vOffset + cy * this.lineHeight;

			return [x, y];
		};

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

		this._uniselect = function(state){
			var state = (typeof(state) == "undefined") ? 
								true : state ? true : false;

			if (state) {
				/* Select All */
				this.setCaret(0, this.text.length);
			} else {
				/* Set Caret, no selection */
				this.setCaret(this.selection.offset, 0);
			}
		}

		this.select = function(state){
			/* 2 times faster than the old while loop method */
			if (this.multiline === false) {
				this._uniselect(state);
				return this;
			}
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
			this.scrollCheck(this.caret.x2, this.caret.y2);			
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
			if (this.selection.size > 0) {
				Native.setPasteBuffer(this.selection.text);
			} else {
				Native.setPasteBuffer("");
			}
		};

		this.paste = function(){
			var pasteBuffer = Native.getPasteBuffer();
			if (pasteBuffer) {
				this.replace(pasteBuffer);
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

			/* manage caret when multiline = false */

			if (this.multiline === false) {
				this.caret = {
					x1 : offset,
					y1 : 0,
					x2 : offset + size,
					y2 : 0
				};
				
				this.selection = {
			 		text : this.text.substr(offset, size),
			 		offset : offset,
			 		size : size
			 	}

				/* select letters between (x1, 0) and (x2, 0) */
				for (x=0, chars = m[0].letters; x<chars.length; x++){
					chars[x].selected = false;
					if (x >= this.caret.x1 && x < this.caret.x2) {
						chars[x].selected = true;
					}
				}

				this.caretCounter = 0;
				this.updateOverlay();

			 	return this.selection;
			}

			/* manage caret when multiline = true */

			/* select letters between (x1, y1) and (x2, y2) */
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
			this.updateOverlay();

		 	return this.selection;
		};

		this.setSelectionFromSingleLineCaret = function(c){
		 	var offset = c.x1,
		 		size = c.x2 - c.x1;

		 	this.setCaret(offset, size);
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
		 		letter.fullWidth = letter.width + nush + 0.25;
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
			this.updateOverlay();
		};

		/* ------------------------------------------------------------------ */
		
		this.updateOverlay = function(){
			var	params = this.getDrawingBounds(),
				x = params.x + this.padding.left,
				y = params.y + this.padding.top,
				w = params.w - this.padding.right - this.padding.left,
				h = params.h - this.padding.top - this.padding.bottom,

				vOffset = (this.lineHeight/2)+5;

			if (this.multiline) {
				printTextOverlay(
					this,
					x,
					y,
					vOffset, 
					w,
					h, 
					params.y
				);
			} else {
				printLineOverlay(this, x, y, vOffset);
			}
		};

		this.setText(this.text);
		this.resetStartPoint();
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

		context.fontSize = this.fontSize;
		context.fontType = this.fontType;

		if (this.__startTextSelectionProcessing && this._autoScrollTop && this._autoScrollTop !== false) {
			this.parent.updateScrollLeft(this._autoScrollTop);
			this._doMouseSelection({
				x : window.mouseX,
				y : window.mouseY
			});
		}

		printTextMatrix(
			this,
			x,
			y,
			vOffset, 
			w,
			h, 
			params.y
		);

	}
});

function getUnilineLetters(context, wordsArray, fontSize){
	var widthOf = context.measureText,
		textLine = wordsArray.join(' '),
		__cache = [],
		position = 0,
		letters = [];

	// cache width of letters
	var cachedLetterWidth = function(char){
		return __cache[char] ? __cache[char] : __cache[char] = widthOf(char);
	}

	context.setFontSize(fontSize);

	for (var i=0; i<textLine.length; i++){
		var char = textLine[i],
			letterWidth = Math.floor(cachedLetterWidth(char));

		letters[i] = {
			char : char,
			position : position,
			width : letterWidth,
			linegap : 0,
			selected : false
		};
		position += letterWidth;
	}

	return letters;
}

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

	switch (textAlign){
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
			letterWidth = Math.floor(cachedLetterWidth(char));

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
			position : (offsetLeft + offset + position + offgap),
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

function getTextLine(element){
	var	text = element.text,
		matrix = [],
		wordsArray = text.split(' '),
		context = element.layer.context;

	matrix[0] = {
		text : text,
		align : "left",
		words : wordsArray,
		letters : getUnilineLetters(
			context,
			wordsArray,
			element.fontSize
		)
	};

	return matrix;
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

function printTextMatrix(element, x, y, vOffset, viewportWidth, viewportHeight, viewportTop){
	var letters = [],
		context = element.layer.context,
		textMatrix = element._textMatrix,
		lineHeight = element.lineHeight,
		fontSize = element.fontSize,
		fontType = element.fontType,
		color = element.color;

	context.fontSize = fontSize;
	context.fontType = fontType;
	context.setColor(color);

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
			context.drawLetters(letters, tx, ty);
		}
	}
}


function printLineOverlay(element, x, y, vOffset){
	var	context = element.overlay.layer.context,
		textMatrix = element._textMatrix,
		letters = textMatrix[0].letters,
		caret = element.caret,
		lineHeight = element.lineHeight,
		caretOpacity = element.caretOpacity;

	var tx = parseFloat(x),
		ty = parseFloat(y + vOffset);

	element.overlay.layer.clear();
	context.setColor("rgba(180, 180, 255, 0.40)");
	context.highlightLetters(letters, tx, ty - vOffset, lineHeight);

	if (caretOpacity >= 0.10) {
		context.setColor("rgba(0, 0, 0, 0.90)");
		context.drawCaret(
			letters, 
			tx, ty, 
			lineHeight, vOffset, caret.x1, caretOpacity
		);
	}
}


function printTextOverlay(element, x, y, vOffset, viewportWidth, viewportHeight, viewportTop){
	var letters = [],
		context = element.overlay.layer.context,
		textMatrix = element._textMatrix,
		caret = element.caret,
		lineHeight = element.lineHeight,
		caretOpacity = element.caretOpacity;

	var start = 0,
		end = textMatrix.length;

	element.overlay.layer.clear();
	context.setColor("rgba(180, 180, 255, 0.40)");

	if (start-1 >= 0) start--;
	if (end+1 <= textMatrix.length) end++;

	for (var line=start; line<end; line++){
		var tx = x,
			ty = y + vOffset + lineHeight * line;

		letters = textMatrix[line].letters;
		context.highlightLetters(letters, tx, ty - vOffset, lineHeight);
	}

	if (textMatrix[caret.y2] && caretOpacity >= 0.10) {
		line = caret.y2;
		tx = x,
		ty = y + vOffset + lineHeight * line;
		letters = textMatrix[line].letters;
		context.setColor("rgba(0, 0, 0, 0.90)");
		context.drawCaret(
			letters, 
			tx, ty, 
			lineHeight, vOffset, caret.x2, caretOpacity
		);
	}

}






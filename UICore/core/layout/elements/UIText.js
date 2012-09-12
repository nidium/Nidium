/* -------------------------- */
/* Native (@) 2012 Stight.com */
/* -------------------------- */

UIElement.extend("UIText", {
	init : function(){

		this._area = null;
		this.flags._canReceiveFocus = true;

		this.addEventListener("mouseover", function(e){
			this.verticalScrollBar.fadeIn(150, function(){
				console.log("done");
			});
		}, false);

		this.addEventListener("mouseout", function(e){
			this.verticalScrollBar.fadeOut(400, function(){
				console.log("done");
			});
		}, false);

		this.addEventListener("dragstart", function(e){
			this.__startx = e.x;
			this.__starty = e.y + this.scroll.top;
		}, false);

		this.addEventListener("dragover", function(e){
			this.area = {
				x1 : this.__startx,
				y1 : this.__starty,
				x2 : e.x,
				y2 : e.y + this.scroll.top
			};
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

		/* --------------------- */

		this.addEventListener("mousewheel", function(e){
			if (this.h / this.scrollBarHeight < 1) {
				canvas.__mustBeDrawn = true;
				this.scrollY(1 + (-e.yrel-1) * 18);
			}
		}, false);

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
			h = params.h - pad.top - pad.bottom;



		//if (this.__cache) {
			//canvas.putImageData(this.__cache, params.x, params.y);
		//} else {
			var off = new Canvas(64, 64);

			canvas.save();
				canvas.roundbox(params.x, params.y, params.w, params.h, this.radius, this.background, false); // main view
				canvas.clip();
		
				canvas.fillStyle = "#000000";
				canvas.fontSize = this.fontSize;
				printAtWordWrap(this.text, x, y, this.scroll.top, 20, w, h, "justify", this.area, this);

			canvas.restore();
			//this.__cache = canvas.getImageData(params.x, params.y, params.w, params.h);
		//}

	}
});

canvas.implement({
	drawWordsArray : function(wordsArray, x, y, textAlign, fitWidth, lastLine){
		var deltawidth = this.measureText(" "),
			textLine = wordsArray.join(' '),
			nb_words = wordsArray.length,
			linegap = fitWidth - this.measureText(textLine),
			zx = x;

		switch(textAlign) {
			case "justify" :
				if (!lastLine) {
					deltawidth += ( linegap / nb_words );
				}
				break;
			case "right" :
				zx += linegap;
				break;
			case "center" :
				zx += linegap/2;
				break;
			default :
				break;
		}

		for (var i=0 ; i<nb_words ; i++){
			this.fillText(wordsArray[i], zx, y);
			zx += Math.round(( this.measureText( wordsArray[i] )   + deltawidth)*100)/100 ;
		}

	}
});

function printAtWordWrap(text, x, y, scrollTop, lineHeight, fitWidth, fitHeight, textAlign, area, UIView){
	fitWidth = fitWidth || 0;
	lineHeight = lineHeight || 20;

	var vOffset = (lineHeight/2)+4,
		currentLine = 0,
		context = canvas,
		spaceWidth = context.measureText(" "),
		paragraphe = text.split(/\r\n|\r|\n/),

		wordsArray = [],
		offsetY = 0,
		line_start = 0,
		line_end = paragraphe.length;

	y = y - scrollTop;

	// cut overflow
	//var ls = Math.round(scrollTop / lineHeight);
	//echo(ls);

	for (var i = 0; i < paragraphe.length; i++) {
		var words = paragraphe[i].split(' '),
			idx = 1;

		while (words.length > 0 && idx <= words.length) {
			var str = words.slice(0, idx).join(' '),
				w = context.measureText(str);

			if (w > fitWidth) {
				idx = idx == 1 ? 2 : idx;

				wordsArray = words.slice(0, idx - 1);
				
				// only draw visible lines
				offsetY = lineHeight * currentLine;
				if ( (y + offsetY + vOffset) < (UIView._y + fitHeight + lineHeight) &&  (y + offsetY + vOffset) >= (UIView._y)) {
					context.drawWordsArray(wordsArray, x, y + offsetY + vOffset, textAlign, fitWidth);
				}

				currentLine++;

				words = words.splice(idx - 1);
				idx = 1;

			} else {
				idx++;
			}

		}

		// last line
		if (idx > 0) {
			context.drawWordsArray(words, x, y + (lineHeight * currentLine) + vOffset, textAlign, fitWidth, true);
		}

		currentLine++;
	}

	UIView.content.height = currentLine*lineHeight;

	if (area) {

		if (area.y2 < UIView.__starty) {
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
				x1 : area.x1 - x,
				y1 : Math.floor((area.y1 - (y+UIView.scroll.top) ) / lineHeight) * lineHeight,
				x2 : area.x2 - x,
				y2 : Math.ceil((area.y2 - (y+UIView.scroll.top)) / lineHeight) * lineHeight,
			};

		var rw = r.x2 - r.x1,
			rh = r.y2 - r.y1,
			nb_lines = rh/lineHeight;

		
		//context.fillStyle = "rgba(255, 0, 0, 0.25)";
		//context.fillRect(x+r.x1 , y+r.y1 + UIView.scroll.top , rw, rh);

		context.fillStyle = "rgba(180, 180, 255, 0.50)";

		if (nb_lines<=1) {
				context.fillRect(x+r.x1, y + r.y1, rw, lineHeight);
		} else {

			for (var i = 0; i < nb_lines; i++) {
				
				var offsetY = i * lineHeight;

				if (i==0) {
					context.fillRect(x+r.x1, y + r.y1 + offsetY, fitWidth - r.x1, lineHeight);
				} else if (i==nb_lines-1) {
					context.fillRect(x , y + r.y1 + offsetY, r.x2, lineHeight);
				} else {
					context.fillRect(x , y + r.y1 + offsetY, fitWidth, lineHeight);
				}

			};
		}

	}

}


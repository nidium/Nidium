/* ------------------------+------------- */
/* Native Framework 2.0    | Falcon Build */
/* ------------------------+------------- */
/* (c) 2013 nidium.com - Vincent Fontaine */
/* -------------------------------------- */

Native.elements.export("UIView", {
	public : {
		backgroundImage : {
			value : function(){
				return OptionalString(this.options.backgroundImage, '');
			},

			set : function(value){
				if (value) {
					this.setBackgroundURL(value);
				}
			}
		},
		
		scrollLeft : {
			set : function(value){
				var max = this.scrollable ?
								this.layer.contentWidth - this._width : 0;

				this.scrollLeft = Math.min(value, max);
				this.refreshScrollBars();
			}
		},
		
		scrollTop : {
			set : function(value){
				var max = this.scrollable ?
								this.layer.contentHeight - this._height : 0;

				this.scrollTop = Math.min(value, max);
				this.refreshScrollBars();
			}
		}
	},

	onChildReady : function(child){
		/* each time a child is added, bringToFront scrollbars */
		if (this.VScrollBar && this.HScrollBar){
			this.VScrollBar.bringToFront();
			this.HScrollBar.bringToFront();
		}
	},

	init : function(){
		var self = this,
			scrollBarHideDelay = 1500,
			o = this.options;

		if (this.scrollable === true){
			this.scrollBarX = o.scrollBarX === false ? false : true;
			this.scrollBarY = o.scrollBarY === false ? false : true;
		}

		this.setBackgroundURL = function(url){
			if (url) {
				self._cachedBackgroundImage = null;
				this._backgroundImage = url;
				Image.load(url, function(img){
					self.setBackgroundImage(img);
				});
			}
		};

		this.setBackgroundImage = function(img){
			self._cachedBackgroundImage = img;
			self._needRefresh = true;
			self._needRedraw = true;
			self.__refresh();
		};

		/* -- SCROLL RELATED METHODS ---------------------------------------- */

		this.spacedrag = true;
		this.__scrolldir__ = 1;

		this.invertScrollDirection = function(){
			this.__scrolldir__ = -this.__scrolldir__;
		};

		this.getMaxScrollTop = function(){
			return this.scrollable ? this.layer.contentHeight - this.height : 0;
		};

		this.getMaxScrollLeft = function(){
			return this.scrollable ? this.layer.contentWidth - this.width : 0;
		};

		var showScrollBar = function(UIScrollBar){
			UIScrollBar.finishCurrentAnimations("opacity");
			UIScrollBar._fading = false;
			UIScrollBar.opacity = 1;
			UIScrollBar.show();
		};

		var hideScrollBar = function(UIScrollBar){
			UIScrollBar.fadeOut(250, function(){
				this.hide();
				this._fading = false;
			});
		};

		this.startFadeOutTimer = function(UIScrollBar){
			if (UIScrollBar._fading) return false;
			UIScrollBar._fading = true;

			clearTimeout(UIScrollBar.fadeOutTimer);
			
			UIScrollBar.fadeOutTimer = setTimeout(function(){
				hideScrollBar(UIScrollBar);
			}, scrollBarHideDelay);
		};

		this.killFadeOutTimer = function(UIScrollBar){
			clearTimeout(UIScrollBar.fadeOutTimer);
		};

		this.animateScrollTop = function(dy){
			var UIScrollBar = this.VScrollBar,
				val = this._scrollTop,
				max = this.getMaxScrollTop();

			if (val === 0 && max === 0) {
				// not scrollable, forward ...
				return false;
			}

			if (dy>0) {
				if (val-dy <= 0) {
					// try to scroll above top, forward ...
					this.scrollTop = 0;
					return false;
				}
			} else {
				if (val-dy >= max) {
					// try to scroll bellow bottom, forward ...
					this.scrollTop = max;
					return false;
				}
			}

			if (this.height < this.contentHeight) {
				showScrollBar(UIScrollBar);
				this.__animateScrollY(-dy * 4, function(){
					self.startFadeOutTimer(UIScrollBar);
				});
			}

			return true;
		};

		this.animateScrollLeft = function(dx){
			var UIScrollBar = this.HScrollBar,
				val = this._scrollLeft,
				max = this.getMaxScrollLeft();

			if (val === 0 && max === 0) {
				// not scrollable, forward ...
				return false;
			}

			if (dx>0) {
				if (val-dx <= 0) {
					// try to scroll beyond left limit, forward ...
					this.scrollLeft = 0;
					return false;
				}
			} else {
				if (val-dx >= max) {
					// try to scroll beyond right limit, forward ...
					this.scrollLeft = max;
					return false;
				}
			}
			
			if (this.width < this.contentWidth) {
				showScrollBar(UIScrollBar);
				this.__animateScrollX(-dx * 4, function(){
					self.startFadeOutTimer(UIScrollBar);
				});
			}

			return true;
		};

		/* incremental scrollLeft */
		this.dxScroll = function(dx, smooth, scaling=1){
			var	vw = this._width,
				sw = this.contentWidth,
				scale = sw/vw,
				max = sw-vw,
				inc = dx*scale*scaling;

			if (smooth) {
				this.__animateScrollX(inc);
			} else {
			
				if (this.scrollLeft+inc >= 0 && this.scrollLeft+inc <= max){
					this.scrollLeft += inc;
				} else {
					if (inc !== 0) this.scrollLeft = (inc<0) ? 0 : max;
				}

			}

			return this;
		};

		/* incremental scrollTop */
		this.dyScroll = function(dy, smooth, scaling=1){
			var vh = this._height,
				sh = this.contentHeight,
				scale = sh/vh,
				max = sh-vh,
				inc = dy*scale*scaling;

			if (smooth) {
				this.__animateScrollY(inc);
			} else {

				if (this.scrollTop+inc >= 0 && this.scrollTop+inc <= max){
					this.scrollTop += inc;
				} else {
					if (inc !== 0) this.scrollTop = (inc<0) ? 0 : max;
				}

			}
		};

		/* set content scroll to relative (dx, dy) - no animation */
		this.scrollBy = function(dx, dy){
			this.dxScroll(dx, false);
			this.dyScroll(dy, false);
			return this;
		};

		/* animate content scroll to relative (dx, dy) */
		this.slideBy = function(dx, dy, scaling){
			this.dxScroll(dx, true, scaling);
			this.dyScroll(dy, true, scaling);
			return this;
		};

		/* set content scroll to absolute (x, y) - no animation */
		this.scrollTo = function(x, y){
			var dx = x-this.scrollLeft,
				dy = y-this.scrollTop;
			this.dxScroll(dx, false);
			this.dyScroll(dy, false);
			return this;
		};

		/* animate content scroll to absolute (x, y) */
		this.slideTo = function(x, y){
			var dx = x-this.scrollLeft,
				dy = y-this.scrollTop;
			this.dxScroll(dx, true);
			this.dyScroll(dy, true);
			return this;
		};

		this.__animateScrollY = function(delta, callback){
			var self = this,
				fn = OptionalCallback(callback),
				max = this.getMaxScrollTop(),
				slice = 10,
				step = 5,
				dec = 0.98;

			if (!self._scrollYinitied) {
				self._scrollYinitied = true;
				self._scrollYgoal = self._scrollTop + delta;
				self._scroll_dy = 1
			} else {
				/* set new goal */
				//self._scrollTop = Math.round(self._scrollYgoal*10)/10;
				self._scrollYgoal += delta;
			}

			/* Scroll Velocity Interpolation */
			if (delta>0) {
				self._scroll_dy = (self._scrollYgoal - self._scrollTop)/step;
			} else {
				self._scroll_dy = (self._scrollTop - self._scrollYgoal)/step;
			}

			if (self._scrollYtimer) {
				self._scrollYtimer.remove();
			}

			/* Scroll to goal and slowdown velocity */
			self._scrollYtimer = window.timer(function(){
				var stop = false,
					value = self._scrollTop;

				if (delta>0){
					if (value < self._scrollYgoal){
						value += self._scroll_dy;
						self._scroll_dy *= dec;
					} else {
						value = self._scrollYgoal;
						stop = true;
					}
				} else {
					if (value > self._scrollYgoal){
						value += -self._scroll_dy;
						self._scroll_dy *= dec;
					} else {
						value = self._scrollYgoal;
						stop = true;
					}

				} 

				/* stop below bottom */
				if (value > max) {
					value = max;
					stop = true;
				}

				/* stop above top */
				if (value < 0) {
					value = 0;
					stop = true;
				}

				if (stop){
					self._scrollYinitied = false;
					this.remove();
					fn.call(self);
				}

				/* subpixel precision */
				self.scrollTop = Math.round(value*10)/10;

			}, slice, true, true);

			return true;
		};

		this.__animateScrollX = function(delta, callback){
			var self = this,
				fn = OptionalCallback(callback),
				max = self.contentWidth - self._width,
				slice = 10,
				step = 5,
				dec = 0.98;

			if (!self._scrollXinitied) {
				self._scrollXinitied = true;
				self._scrollXgoal = self._scrollLeft + delta;
				self._scroll_dx = 1;
			} else {
				self._scrollXgoal += delta;
			}

			if (delta>0) {
				self._scroll_dx = (self._scrollXgoal - self._scrollLeft)/step;
			} else {
				self._scroll_dx = (self._scrollLeft - self._scrollXgoal)/step;
			}

			if (self._scrollXtimer) {
				self._scrollXtimer.remove();
			}

			self._scrollXtimer = window.timer(function(){
				var stop = false,
					value = self._scrollLeft;

				if (delta>0){
					if (value < self._scrollXgoal){
						value += self._scroll_dx;
						self._scroll_dx *= dec;
					} else {
						value = self._scrollXgoal;
						stop = true;
					}
				} else {
					if (value > self._scrollXgoal){
						value += -self._scroll_dx;
						self._scroll_dx *= dec;
					} else {
						value = self._scrollXgoal;
						stop = true;
					}

				} 

				if (value > max){
					value = max;
					stop = true;
				}

				if (value < 0){
					value = 0;
					stop = true;
				}

				if (stop){
					self._scrollXinitied = false;
					this.remove();
					fn.call(self);
				}

				self.scrollLeft = Math.round(value*10)/10;

			}, slice, true, true);

			return true;
		};

		this.refreshVerticalScrollBar = function(){
			if (!this.VScrollBar || !this.VScrollBarHandle) return false;

			var container = this.VScrollBar,
				handle = this.VScrollBarHandle,
				ch = container._height,
				sh = this.contentHeight,
				viewWidth = this._width,
				viewHeight = this._height,
				radius = this._radius,
				scale = ch/sh,
				maxScrollTop = Math.min(this._scrollTop, sh - viewHeight),
				offset = 0;


			if (this._scrollTop > (sh - viewHeight)){
				this._scrollTop = Math.max(0, sh - viewHeight);
			}


			if (container.hidden === false) {
				offset = (this.HScrollBar && this.HScrollBar.visible) ? 8 : 0;

				container.left = viewWidth - 8 - radius;
				container.top = radius;
				container.height = viewHeight - 2*radius - offset;

				handle.top = Math.round(maxScrollTop * scale);
				handle.height = Math.round(viewHeight * scale);
			}
		};

		this.refreshHorizontalScrollBar = function(){
			if (!this.HScrollBar || !this.HScrollBarHandle) return false;

			var container = this.HScrollBar,
				handle = this.HScrollBarHandle,
				cw = container._width,
				sw = this.contentWidth,
				viewWidth = this._width,
				viewHeight = this._height,
				radius = this._radius,
				scale = cw/sw,
				maxScrollLeft = Math.min(this._scrollLeft, sw - viewWidth),
				offset = 0;

			if (this._scrollLeft > (sw - viewWidth)){
				this._scrollLeft = Math.max(0, sw - viewWidth);
			}

			if (container.hidden === false) {
				offset = (this.VScrollBar && this.VScrollBar.visible) ? 8 : 0;

				container.left = radius;
				container.top = viewHeight - 8 - radius;
				container.width = viewWidth - 2*radius - offset;

				handle.left = Math.round(maxScrollLeft * scale);
				handle.width = Math.round(viewWidth * scale);
			}
		};

		this.refreshScrollBars = function(){
			this.refreshVerticalScrollBar();
			this.refreshHorizontalScrollBar();
		};

		this.createVerticalScrollBar = function(){
			if (this.VScrollBar) return false;
			this.VScrollBar = this.add("UIScrollBar", {
				position : "fixed",
				width : 8,
				height : this._height,
				left : this._width - 8,
				top : 0,
				hidden : !this.scrollBarY
			});
			
			this.VScrollBarHandle = this.VScrollBar.add(
				"UIScrollBarHandle", {
					width : 8,
					height : this.VScrollBar._height
				}
			);

			this.VScrollBarHandle.addEventListener("dragstart", function(e){
				showScrollBar(self.VScrollBar);
				self.killFadeOutTimer(self.VScrollBar);
				e.stopPropagation();
			}, false);

			this.VScrollBarHandle.addEventListener("drag", function(e){
				self.dyScroll(e.y - this.__top - this.height/2, false);
				e.stopPropagation();
			}, false);

			this.VScrollBarHandle.addEventListener("dragend", function(e){
				self.startFadeOutTimer(self.VScrollBar);
				e.stopPropagation();
			}, false);

		};

		this.createHorizontalScrollBar = function(){
			if (this.HScrollBar) return false;
			this.HScrollBar = this.add("UIScrollBar", {
				position : "fixed",
				width : this._width,
				height : 8,
				left : 0,
				top : this._height - 8,
				hidden : !this.scrollBarX
			});
			
			this.HScrollBarHandle = this.HScrollBar.add(
				"UIScrollBarHandle", {
					width : this.HScrollBar._width,
					height : 8
				}
			);

			this.HScrollBarHandle.addEventListener("dragstart", function(e){
				showScrollBar(self.HScrollBar);
				self.killFadeOutTimer(self.HScrollBar);
				e.stopPropagation();
			}, false);

			this.HScrollBarHandle.addEventListener("drag", function(e){
				self.dxScroll(e.x - this.__left - this.width/2, false);
				e.stopPropagation();
			}, false);

			this.HScrollBarHandle.addEventListener("dragend", function(e){
				self.startFadeOutTimer(self.HScrollBar);
				e.stopPropagation();
			}, false);

		};

		if (this.scrollable === true){
			this.createVerticalScrollBar();
			this.createHorizontalScrollBar();
			this.refreshScrollBars();
		}

		this.addEventListener("dragstart", function(e){
			if (!this.scrollable || !this.spacedrag || !e.spaceKeyDown) {
				e.forcePropagation();
				return true;
			}

			this.dragging = true;
			showScrollBar(self.HScrollBar);
			self.killFadeOutTimer(self.HScrollBar);

			showScrollBar(self.VScrollBar);
			self.killFadeOutTimer(self.VScrollBar);
			e.stopPropagation();
		}, false);

		this.addEventListener("drag", function(e){
			if (!this.dragging) {
				e.forcePropagation();
				return true;
			}

			this.slideBy(
				e.xrel * this.__scrolldir__,
				e.yrel * this.__scrolldir__,
				0.5
			);

			showScrollBar(self.HScrollBar);
			showScrollBar(self.VScrollBar);
			e.stopPropagation();
		}, false);

		this.addEventListener("dragend", function(e){
			if (!this.dragging) {
				e.forcePropagation();
				return true;
			}

			this.dragging = false;
			self.startFadeOutTimer(self.HScrollBar);
			self.startFadeOutTimer(self.VScrollBar);
			e.stopPropagation();
		}, false);


		this.addEventListener("mousewheel", function(e){
			var stop = false;
			if (this.VScrollBar && e.yrel != 0){
				stop = this.animateScrollTop(e.yrel * this.__scrolldir__);
			}

			if (this.HScrollBar && e.xrel != 0){
				stop = this.animateScrollLeft(e.xrel * this.__scrolldir__);
			}

			if (stop) e.stopPropagation();
		}, false);

		this.addEventListener("mouseover", function(e){
			this.refreshScrollBars();
			e.stopPropagation();
		}, false);

		this.addEventListener("mouseout", function(e){
			this.refreshScrollBars();
			e.stopPropagation();
		}, false);

		this.setBackgroundURL(this.backgroundImage);
	},

	draw : function(context){
		var	params = this.getDrawingBounds();

		if (this.shadowBlur != 0) {
			context.setShadow(
				this.shadowOffsetX,
				this.shadowOffsetY,
				this.shadowBlur,
				this.shadowColor
			);
		}

		NDMElement.draw.box(this, context, params);
		context.setShadow(0, 0, 0);

		if (this._cachedBackgroundImage) {
			if (this._backgroundRepeat === false){
				context.save();
					context.clipbox(
						params.x, params.y,
						params.w, params.h,
						this.radius
					);
					context.clip();
		
					context.drawImage(
						this._cachedBackgroundImage,
						params.x, params.y
					);
				context.restore();
			} else {
				var pattern = context.createPattern(
					this._cachedBackgroundImage,
					"repeat"
				);
				NDMElement.draw.box(this, context, params, pattern);
			}
		}
		/*
		if (this.matrix){
			var x = params.x + this.paddingLeft,
				y = params.y + this.paddingTop,
				w = params.w - this.paddingRight - this.paddingLeft,
				h = params.h - this.paddingTop - this.paddingBottom,

				vOffset = (this.lineHeight/2)+5;

			this.color = "#555555";
			context.fontSize = this.fontSize;
			context.fontFamily = this.fontFamily;

			printTextMatrix(
				context,
				this._tmpMatrix,
				null,
				x, y - this.parent ? this.parent.scrollTop : 0, 
				vOffset, 
				w, h, 
				params.y, 
				this.lineHeight,
				this.fontSize,
				this.fontFamily,
				this.color, 
				this.caretOpacity
			);
		}
		*/

	}
});

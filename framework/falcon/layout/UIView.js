/* ------------------------+------------- */
/* Native Framework 2.0    | Falcon Build */
/* ------------------------+------------- */
/* (c) 2013 Stight.com - Vincent Fontaine */
/* -------------------------------------- */

Native.elements.export("UIView", {
	public : {
		backgroundImage : {
			value : function(){
				return OptionalString(this.options.backgroundImage, '');
			},

			set : function(value){
				this.setBackgroundURL(value);
			}
		},

		contentWidth : {
			set : function(value){
				throw "contentWidth is read only (id:"+this.id+")";
			},

			get : function(){
				return this.layer.contentWidth;
			}
		},

		contentHeight : {
			set : function(value){
				throw "contentHeight is read only (id:"+this.id+")";
			},

			get : function(){
				return this.layer.contentHeight;
			}
		},

		scrollLeft : {
			set : function(value){
				var max = this.scrollbars ?
								this.layer.contentWidth - this._width : 0;

				this.scrollLeft = Math.min(value, max);
				this.refreshScrollBars();
			}
		},
		
		scrollTop : {
			set : function(value){
				var max = this.scrollbars ?
								this.layer.contentHeight - this._height : 0;

				this.scrollTop = Math.min(value, max);
				this.refreshScrollBars();
			}
		}
	},

	init : function(){
		var self = this,
			scrollBarHideDelay = 400,
			o = this.options;

		this.setBackgroundURL = function(url){
			if (url) {
				self._cachedBackgroundImage = null;
				this._backgroundImage = url;
				Native.loadImage(url, function(img){
					self.setBackgroundImage(img);
				});
			}
		};

		this.setBackgroundImage = function(img){
			self._cachedBackgroundImage = img;
			self._needRefresh = true;
			self._needRedraw = true;
			self.refresh();
		};

		this.getMaxScrollTop = function(){
			return this.scrollbars ? this.layer.contentHeight - this.height : 0;
		};

		var showScrollBar = function(UIScrollBar){
			UIScrollBar.finishCurrentAnimations("opacity");
			UIScrollBar._fading = false;
			UIScrollBar.opacity = 1;
			UIScrollBar.show();
			UIScrollBar.bringToFront();
		};

		var hideScrollBar = function(UIScrollBar){
			UIScrollBar.fadeOut(250, function(){
				this.hide();
				this.sendToBack();
				this._fading = false;
			});
		};

		var scheduler = function(timer, UIScrollBar){
			if (UIScrollBar._fading) return false;

			UIScrollBar._fading = true;
			
			clearTimeout(self[timer]);
			
			self[timer] = setTimeout(function(){
				hideScrollBar(UIScrollBar);
			}, scrollBarHideDelay);
		};

		this.updateScrollTop = function(dy){
			var UIScrollBar = this.VScrollBar;

			if (this.height / this.contentHeight < 1) {
				showScrollBar(UIScrollBar);
				this.scrollContentY(-dy * 4, function(){
					scheduler("_scrollYfadeTimer", UIScrollBar);
				});
			}
		};

		this.updateScrollLeft = function(dx){
			var UIScrollBar = this.HScrollBar;

			if (this.width / this.contentWidth < 1) {
				showScrollBar(UIScrollBar);
				this.scrollContentX(-dx * 4, function(){
					scheduler("_scrollXfadeTimer", UIScrollBar);
				});
			}
		};

		this.scrollContentY = function(delta, callback){
			var self = this,
				fn = OptionalCallback(callback),
				max = self.contentHeight - self._height,
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
			self._scrollYtimer = Native.timer(function(){
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

		this.scrollContentX = function(delta, callback){
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

			self._scrollXtimer = Native.timer(function(){
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
			print("refreshVerticalScrollBar", this);

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

/*
			if (scale == 1){
				container.visible = false;
				handle.visible = false;
			} else {
				container.visible = true;
				handle.visible = true;
			}
*/

			if (this._scrollTop > (sh - viewHeight)){
				this._scrollTop = Math.max(0, sh - viewHeight);
			}

			offset = (this.HScrollBar && this.HScrollBar.visible) ? 8 : 0;

			container.left = viewWidth - 8 - radius;
			container.top = radius;
			container.height = viewHeight - 2*radius - offset;

			handle.top = Math.round(maxScrollTop * scale);
			handle.height = Math.round(viewHeight * scale);
		};

		this.refreshHorizontalScrollBar = function(){
			if (!this.HScrollBar || !this.HScrollBarHandle) return false;
			print("refreshHorizontalScrollBar", this);

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

/*
			if (scale == 1){
				container.visible = false;
				handle.visible = false;
			} else {
				container.visible = true;
				handle.visible = true;
			}
*/
			offset = (this.VScrollBar && this.VScrollBar.visible) ? 8 : 0;

			if (this._scrollLeft > (sw - viewWidth)){
				this._scrollLeft = Math.max(0, sw - viewWidth);
			}

			container.left = radius;
			container.top = viewHeight - 8 - radius;
			container.width = viewWidth - 2*radius - offset;

			handle.left = Math.round(maxScrollLeft * scale);
			handle.width = Math.round(viewWidth * scale);
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
				top : 0
			});
			
			this.VScrollBarHandle = this.VScrollBar.add(
				"UIScrollBarHandle", {
					width : 8,
					height : this.VScrollBar._height
				}
			);
		};

		this.createHorizontalScrollBar = function(){
			if (this.HScrollBar) return false;
			this.HScrollBar = this.add("UIScrollBar", {
				position : "fixed",
				width : this._width,
				height : 8,
				left : 0,
				top : this._height - 8
			});
			
			this.HScrollBarHandle = this.HScrollBar.add(
				"UIScrollBarHandle", {
					width : this.HScrollBar._width,
					height : 8
				}
			);
		};

		if (this.scrollbars === true){
			this.createVerticalScrollBar();
			this.createHorizontalScrollBar();
			this.refreshScrollBars();
		}

		this.addEventListener("mousewheel", function(e){
			var stop = false;
			if (this.VScrollBar && e.yrel != 0){
				this.updateScrollTop(e.yrel);
				stop = true;
			}

			if (this.HScrollBar && e.xrel != 0){
				this.updateScrollLeft(e.xrel);
				stop = true;
			}

			if (stop) e.stopPropagation();
		}, false);

		this.addEventListener("mouseover", function(e){
			this.refreshScrollBars();
		});

		this.addEventListener("mouseout", function(e){
			this.refreshScrollBars();
		});

		this.setBackgroundURL();
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

		DOMElement.draw.box(this, context, params);
		context.setShadow(0, 0, 0);

		if (this._cachedBackgroundImage) {
			if (true === false){
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
				DOMElement.draw.box(this, context, params, pattern);
			}
		}
	}
});

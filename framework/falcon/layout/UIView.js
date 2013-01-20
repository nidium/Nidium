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
				this.refreshBackgroundImage();
			}
		},

		contentWidth : {
			set : function(value){
				throw "contentWidth is read only (id:"+this.id+")";
			},

			get : function(){
				if (this._cachedContentWidth === null){
					Native.layout.slowUpdateInnerContentSize(this);
				}
				return this._cachedContentWidth;
			}
		},

		contentHeight : {
			set : function(value){
				throw "contentHeight is read only (id:"+this.id+")";
			},

			get : function(){
				if (this._cachedContentHeight === null){
					Native.layout.slowUpdateInnerContentSize(this);
				}
				return this._cachedContentHeight;
			}
		},

		scrollLeft : {
			set : function(value){
				var max = this.scrollbars ?
								this._contentWidth - this._width : 0;

				this.scrollLeft = Math.min(value, max);
				this.refreshScrollBars();
			}
		},
		
		scrollTop : {
			set : function(value){
				var max = this.scrollbars ?
								this._contentHeight - this._height : 0;

				this.scrollTop = Math.min(value, max);
				this.refreshScrollBars();
			}
		}
	},

	init : function(){
		var self = this,
			o = this.options;

		this.refreshBackgroundImage = function(){
			self._cachedBackgroundImage = null;

			if (this.backgroundImage) {
				Native.loadImage(this.backgroundImage, function(img){
					self._cachedBackgroundImage = img;
					self._needRedraw = true;
					self.refresh();
				});
			}
		};

		this.updateScrollTop = function(dy){
			if (this.height / this.contentHeight < 1) {
				this.VScrollBar.cancelCurrentAnimations("opacity");
				this._scrollYfading = false;
				this.VScrollBar.opacity = 1;
				this.VScrollBar.show();
	
				this.scrollContentY(-dy * 4, function(){

					if (!self._scrollYfading) {
						self._scrollYfading = true;
						clearTimeout(self._scrollYfadeScheduler);
						self._scrollYfadeScheduler = setTimeout(function(){
							self.VScrollBar.fadeOut(250, function(){
								self._scrollYfading = false;
							});
						}, 100);
					}

				});
			}
		};

		this.updateScrollLeft = function(dx){
			if (this.width / this.contentWidth < 1) {
				this.HScrollBar.cancelCurrentAnimations("opacity");
				this._scrollXfading = false;
				this.HScrollBar.opacity = 1;
				this.HScrollBar.show();
	
				this.scrollContentX(-dx * 4, function(){

					if (!self._scrollXfading) {
						self._scrollXfading = true;
						clearTimeout(self._scrollXfadeScheduler);
						self._scrollXfadeScheduler = setTimeout(function(){
							self.HScrollBar.fadeOut(250, function(){
								self._scrollXfading = false;
							});
						}, 100);
					}
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

				scale = ch/sh,
				maxScrollTop = Math.min(this._scrollTop, sh - this._height);

			if (scale == 1){
				container.visible = false;
				handle.visible = false;
			} else {
				container.visible = true;
				handle.visible = true;
			}

			if (this._scrollTop > (sh - this._height)){
				this._scrollTop = Math.max(0, sh - this._height);
			}

			handle.top = Math.round(maxScrollTop * scale);
			handle.height = Math.round(this._height * scale);
		};

		this.refreshHorizontalScrollBar = function(){
			if (!this.HScrollBar || !this.HScrollBarHandle) return false;
			print("refreshHorizontalScrollBar", this);

			var container = this.HScrollBar,
				handle = this.HScrollBarHandle,
				cw = container._width,
				sw = this.contentWidth,

				scale = cw/sw,
				maxScrollLeft = Math.min(this._scrollLeft, sw - this._width);

			if (scale == 1){
				container.visible = false;
				handle.visible = false;
			} else {
				container.visible = true;
				handle.visible = true;
			}

			if (this._scrollLeft > (sw - this._width)){
				this._scrollLeft = Math.max(0, sw - this._width);
			}

			handle.left = Math.round(maxScrollLeft * scale);
			handle.width = Math.round(this._width * scale);
		};

		this.refreshScrollBars = function(){
			if (!this.VScrollBar || !this.VScrollBarHandle) return false;
			if (!this.HScrollBar || !this.HScrollBarHandle) return false;
			var vs = this.VScrollBar,
				hs = this.HScrollBar;

			vs.left = this._width - 8 - this._radius;
			vs.top = this._radius;
			vs.height = this._height - 2*this._radius - 8;

			hs.left = this._radius;
			hs.top = this._height - 8 - this._radius;
			hs.width = this._width - 2*this._radius - 8;

			this.refreshVerticalScrollBar();
			this.refreshHorizontalScrollBar();
		};

		this.createVerticalScrollBar = function(){
			if (this.VScrollBar) return false;
			this.VScrollBar = this.add("UIScrollBar", {
				fixed : true,
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
				fixed : true,
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

		this.refreshBackgroundImage();
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
			context.save();
				DOMElement.draw.box(this, context, params);
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
		}

	}
});
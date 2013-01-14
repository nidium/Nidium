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
				print("--------------get contentWidth", this);
				if (this._cachedContentWidth === null){
					Native.layout.slowUpdateInnerContentSize(this);
				}
				//this.refreshScrollBars();
				return this._cachedContentWidth;
			}
		},

		contentHeight : {
			set : function(value){
				throw "contentHeight is read only (id:"+this.id+")";
			},

			get : function(){
				print("-------------- get contentHeight", this);
				if (this._cachedContentHeight === null){
					Native.layout.slowUpdateInnerContentSize(this);
				}
				//this.refreshScrollBars();
				return this._cachedContentHeight;
			}
		},

		scrollLeft : {
			set : function(value){
				this.refreshScrollBars();
			}
		},
		
		scrollTop : {
			set : function(value){
				this.refreshScrollBars();
			}
		}
	},

	init : function(){
		var self = this,
			o = this.options;

		this.scrollContentWidth = this._width;
		this.scrollContentHeight = this._height;

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
			if (this.height / this.scrollContentHeight < 1) {
				this.VScrollBar.cancelCurrentAnimations("opacity");
				this._scrollYfading = false;
				this.VScrollBar.opacity = 1;
				this.VScrollBar.show();

				/* TODO : remove when canvas.opacity available */
				this.VScrollBarHandle.show();
				this.VScrollBarHandle.opacity = 1;
				/* TODO : remove when canvas.opacity available */
	
				this.scrollContentY(-dy * 4, function(){

					if (!self._scrollYfading) {
						self._scrollYfading = true;
						clearTimeout(self._scrollYfadeScheduler);
						self._scrollYfadeScheduler = setTimeout(function(){
							/*
							self.VScrollBar.fadeOut(1650, function(){
								self._scrollYfading = false;
							});
							*/
							/* TODO : remove when canvas.opacity available */
							/*
							self.VScrollBarHandle.fadeOut(1850, function(){
								self._scrollYfading = false;
							});
							*/
							/* TODO : remove when canvas.opacity available */
						}, 650);
					}

				});
			}
		};

		this.refreshVerticalScrollBar = function(){
			if (!this.VScrollBar || !this.VScrollBarHandle) return false;
			print("refreshVerticalScrollBar", this);

			var container = this.VScrollBar,
				handle = this.VScrollBarHandle,
				mx = this.contentWidth,
				my = this.contentHeight,
				
				cw = container._width,
				ch = container._height,

				vw = this.scrollContentWidth = Math.max(mx, this._width),
				vh = this.scrollContentHeight = Math.max(my, this._height);
/*
			if (handle._height == vh){
				container.visible = false;
				handle.visible = false;
			} else {
				container.visible = true;
				handle.visible = true;
			}
*/
			handle.top = (this._scrollTop * (ch / vh));
			handle.height = (this._height / vh) * ch;
		};

		this.refreshScrollBars = function(){
			if (!this.VScrollBar || !this.VScrollBarHandle) return false;
			var vs = this.VScrollBar;

			vs.left = this._width - 8;
			vs.top = 0;
			vs.height = this._height;
			this.refreshVerticalScrollBar();
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
					height : this.VScrollBar._height / 2
				}
			);
		};

		if (this.scrollbars === true){
			this.createVerticalScrollBar();
			this.refreshScrollBars();
//			this.VScrollBar.opacity = 0;
//			this.VScrollBarHandle.opacity = 0;
		}

		this.addEventListener("mousewheel", function(e){
			if (this.VScrollBar) {
				this.updateScrollTop(e.yrel);
				e.stopPropagation();
			}
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
			DOMElement.draw.box(this, context, params);
			context.setShadow(0, 0, 0);
		} else {
			DOMElement.draw.box(this, context, params);
		}

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
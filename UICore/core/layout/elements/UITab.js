/* -------------------------- */
/* Native (@) 2012 Stight.com */
/* -------------------------- */

UIElement.extend("UITab", {
	init : function(){
		var self = this;

		this.flags._canReceiveFocus = true;
		this.w = 14 + Math.round(canvas.measureText(this.label)) + 14;
		this.h = 24;
		this.fontSize = 11;
		this.fontType = "arial";
		//this.color = this.options.color ? this.options.color : "#aaaaaa";

		this.addEventListener("mousedown", function(e){
			this.parent.selectTab(this.tabnum);
			e.stopPropagation();
		}, false);

		this.addEventListener("mouseover", function(e){
			this.hover = true;
		});

		this.addEventListener("mouseout", function(e){
			this.hover = false;
		});

		if (this.options.closable) {
			this.w += 16;

			this.closeButton = this.add("UIButtonClose", {
				x : this.w-26,
				y : 6,
				w : 12,
				h : 12,
				color : '#000000',
				background : "rgba(0, 0, 0, 0.3)"
			});

			this.closeButton.addEventListener("mouseclick", function(){
				this.parent.g = {
					x : 0,
					y : this.parent.h/2
				};
				this.parent.bounceScale(0, 120, function(){
					self.parent._removeTab(this.tabnum);
					//this.remove();
				});
			}, false);

		}

		var __startX = 0,
			__endX = 0,
			__currentDragingTabPosition = false;

		this.addEventListener("dragstart", function(e){
			__fireEvent = false;
			__startX = this.left;
			__endX = this.left + this.w;
			__currentDragingTabPosition = self.parent.getPosition(this.tabnum);
		}, false);

		layout.rootElement.addEventListener("dragover", function(e){
			if (__currentDragingTabPosition===false) { return false; }

			var i = __currentDragingTabPosition,
				c = self.parent.getTab(i),
				n = self.parent.getTab(i+1),
				p = self.parent.getTab(i-1);

			if (c.__x + e.xrel < self.parent.__x) {
				c.left = self.parent.left;
			} else if (c.__x + c.__w + e.xrel > self.parent.__x + self.parent.__w) {
				c.left = self.parent.w - c.w;
			} else {
				c.left += e.xrel;
			}

			if (e.xrel>0) {
				if (n) {
					if (c.__x+c.__w > (n.__x+n.__w/2)) {
						self.parent.position = __currentDragingTabPosition+1;
						self.parent.fireEvent("tabswap", {
							tab : self.parent.selection,
							position : __currentDragingTabPosition+1
						});

						n.slideX(__startX, 150, function(){}, FXAnimation.easeOutCubic);
						self.parent.swapTabs(i, i+1);
						__fireEvent = true;

						__currentDragingTabPosition++;

						i = __currentDragingTabPosition;
						c = self.parent.getTab(__currentDragingTabPosition);
						n = self.parent.getTab(i+1);
						p = self.parent.getTab(i-1);

						__startX += p.__w - self.parent.overlap;
						__endX += p.__w - self.parent.overlap;

					}
				} 
			}

			if (e.xrel<0) {
				if (p) {
					if (c.__x < (p.__x+p.__w/2) ) {
						self.parent.position = __currentDragingTabPosition-1;
						self.parent.fireEvent("tabswap", {
							tab : self.parent.selection,
							position : __currentDragingTabPosition-1
						});

						p.slideX(__endX - p.__w, 150, function(){}, FXAnimation.easeOutCubic);
						self.parent.swapTabs(i, i-1);
						__fireEvent = true;

						__currentDragingTabPosition--;
						c = self.parent.getTab(__currentDragingTabPosition);
						i = __currentDragingTabPosition;
						n = self.parent.getTab(i+1);
						p = self.parent.getTab(i-1);

						__startX -= n.__w - self.parent.overlap;
						__endX -= n.__w - self.parent.overlap;

					}
				}
			}

		}, false);

		this.addEventListener("dragend", function(e){
			if (__currentDragingTabPosition===false) { return false; }
			
			let i = __currentDragingTabPosition,
				c = self.parent.getTab(i);

			c.slideX(__startX, 200, function(){});

			__currentDragingTabPosition = false;
			__startX = false;
			__endX = false;

			if (__fireEvent){
				self.parent.fireEvent("tabmove", {
					tab : this.tabnum,
					positions : self.parent.taborder
				});
			}
			__fireEvent = false;

		}, false);

	},

	draw : function(){
		var params = {
				x : this._x,
				y : this._y,
				w : this.w,
				h : this.h
			},

			radius = Math.max(3, this.radius),
			label = this.label,
			textWidth = Math.round(canvas.measureText(label)),
			textHeight = 10,
			w = params.w,
			h = params.h,
			textOffsetX = 15,
			textOffsetY = (h-textHeight)/2 + 9,
			textColor = this.color,
			textShadow = '#000000';
			
		this.shadow = true;
		if (this.shadow) {
			if (this.selected){
				canvas.setShadow(0, 0, 2, this.background);
			} else {
				canvas.setShadow(3, -2, 3, "rgba(0, 0, 0, 0.4)");
			}
		}

		canvas.tabbox(params.x, params.y, w, h, radius, this.background, false);

		if (this.shadow){
			canvas.setShadow(0, 0, 0);
		}

		var gdBackground = canvas.createLinearGradient(params.x, params.y, params.x, params.y+params.h);

		if (this.selected){
			//textOffsetY++;
			textColor = this.color;
			textShadow = "rgba(255, 255, 255, 0.10)";

			gdBackground.addColorStop(0.00, 'rgba(255, 255, 255, 0.20)');
			gdBackground.addColorStop(0.10, 'rgba(255, 255, 255, 0.05)');
			gdBackground.addColorStop(0.90, 'rgba(255, 255, 255, 0.00)');

		} else {

			textColor = this.color;
			textShadow = "rgba(0, 0, 0, 0.35)";

			if (this.hover){
				gdBackground.addColorStop(0.00, 'rgba(255, 255, 255, 0.30)');
				gdBackground.addColorStop(0.15, 'rgba(255, 255, 255, 0.15)');
			} else {
				gdBackground.addColorStop(0.00, 'rgba(255, 255, 255, 0.25)');
				gdBackground.addColorStop(0.10, 'rgba(255, 255, 255, 0.15)');
			}

		}

		canvas.tabbox(params.x, params.y, w, h, radius, gdBackground, false);
		delete(gdBackground);

		canvas.setFontSize(this.fontSize);
		canvas.fontType = this.fontType;

		if (this.hasFocus && this.flags._canReceiveFocus && this.flags._outlineOnFocus) {
			canvas.setColor("rgba(0, 0, 0, 1)");
			canvas.setShadow(0, 0, 3, "rgba(255, 255, 255, 0.4)");
			canvas.fillText(label, params.x+textOffsetX, params.y+textOffsetY);
			canvas.setShadow(0, 0, 0);
		}

//		canvas.setColor(textShadow);
//		canvas.fillText(label, params.x+textOffsetX+1, params.y+textOffsetY+1);

		canvas.setColor(textColor);
		canvas.fillText(label, params.x+textOffsetX, params.y+textOffsetY);

	}
});
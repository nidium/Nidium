/* -------------------------- */
/* Native (@) 2012 Stight.com */
/* -------------------------- */

Native.elements.export("UITab", {
	refresh : function(){
		this._height = 24;
		this._fontSize = 11;
		this._fontType = "arial";
		//this.color = this.options.color ? this.options.color : "#aaaaaa";

		var textWidth = Native.getTextWidth(
			this._label,
			this._fontSize,
			this._fontType
		);

		this._width = 14 + Math.round(textWidth) + 14;

		if (this.options.closable) {
			this._width += 16;
		}		
	},

	init : function(){
		var self = this;

		this.canReceiveFocus = true;

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

		this.onReady = function(){
			if (this.options.closable) {

				this.closeButton = this.add("UIButtonClose", {
					left : this.width - 26,
					top : 6,
					width : 12,
					height : 12,
					color : '#000000',
					background : "rgba(0, 0, 0, 0.3)"
				});

				this.closeButton.addEventListener("mouseup", function(){
					/*
					this.parent.g = {
						x : 0,
						y : this.parent.height/2
					};
					this.parent.bounceScale(0, 120, function(){
						self.parent._removeTab(this.tabnum);
						//this.remove();
					});
					*/
				}, false);

			}

		}

		var __startX = 0,
			__endX = 0,
			__currentDragingTabPosition = false;

		this.addEventListener("dragstart", function(e){
			__fireEvent = false;
			__startX = this.left;
			__endX = this.left + this.width;
			__currentDragingTabPosition = self.parent.getPosition(this.tabnum);
		}, false);

		this._root.addEventListener("dragover", function(e){
			if (__currentDragingTabPosition===false) { return false; }

			var i = __currentDragingTabPosition,
				c = self.parent.getTab(i),
				n = self.parent.getTab(i+1),
				p = self.parent.getTab(i-1);

			if (c._absx + e.xrel < self.parent._absx) {
				c.left = self.parent.left;
			} else if (c._absx + c.width + e.xrel > self.parent._absx + self.parent.width) {
				c.left = self.parent.w - c.w;
			} else {
				c.left += e.xrel;
			}

			if (e.xrel>0) {
				if (n) {
					if (c._absx+c.width > (n._absx+n.width/2)) {
						self.parent.position = __currentDragingTabPosition+1;
						self.parent.fireEvent("tabswap", {
							tab : self.parent.selection,
							position : __currentDragingTabPosition+1
						});

						n.slideX(
							__startX, 
							150, 
							function(){}, 
							Math.physics.cubicOut
						);
						
						self.parent.swapTabs(i, i+1);
						__fireEvent = true;

						__currentDragingTabPosition++;

						i = __currentDragingTabPosition;
						c = self.parent.getTab(__currentDragingTabPosition);
						n = self.parent.getTab(i+1);
						p = self.parent.getTab(i-1);

						__startX += p.width - self.parent.overlap;
						__endX += p.width - self.parent.overlap;

					}
				} 
			}

			if (e.xrel<0) {
				if (p) {
					if (c._absx < (p._absx+p.width/2) ) {
						self.parent.position = __currentDragingTabPosition-1;
						self.parent.fireEvent("tabswap", {
							tab : self.parent.selection,
							position : __currentDragingTabPosition-1
						});

						p.slideX(
							__endX - p.width, 
							150, 
							function(){}, 
							Math.physics.cubicOut
						);

						self.parent.swapTabs(i, i-1);
						__fireEvent = true;

						__currentDragingTabPosition--;
						c = self.parent.getTab(__currentDragingTabPosition);
						i = __currentDragingTabPosition;
						n = self.parent.getTab(i+1);
						p = self.parent.getTab(i-1);

						__startX -= n.width - self.parent.overlap;
						__endX -= n.width - self.parent.overlap;

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

	draw : function(context){
		var	params = this.getDrawingBounds(),
			radius = Math.max(3, this.radius),
			textHeight = 10,
			textOffsetX = 15,
			textOffsetY = (params.h-textHeight)/2 + 9,
			textShadow = '#000000';
			
		if (__ENABLE_BUTTON_SHADOWS__) {
			if (this.selected){
				context.setShadow(0, 0, 2, this.background);
			} else {
				context.setShadow(0, -2, 4, "rgba(0, 0, 0, 0.4)");
			}
		}

		context.tabbox(
			params.x, params.y, 
			params.w, params.h, 
			radius, this.background, false
		);

		if (__ENABLE_BUTTON_SHADOWS__){
			context.setShadow(0, 0, 0);
		}

		var gradient = context.createLinearGradient(
			params.x, params.y, 
			params.x, params.y+params.h
		);

		if (this.selected){
			textShadow = "rgba(255, 255, 255, 0.10)";

			gradient.addColorStop(0.00, 'rgba(255, 255, 255, 0.20)');
			gradient.addColorStop(0.10, 'rgba(255, 255, 255, 0.05)');
			gradient.addColorStop(0.90, 'rgba(255, 255, 255, 0.00)');

		} else {
			textShadow = "rgba(0, 0, 0, 0.35)";

			if (this.hover){
				gradient.addColorStop(0.00, 'rgba(255, 255, 255, 0.30)');
				gradient.addColorStop(0.15, 'rgba(255, 255, 255, 0.15)');
			} else {
				gradient.addColorStop(0.00, 'rgba(255, 255, 255, 0.25)');
				gradient.addColorStop(0.10, 'rgba(255, 255, 255, 0.15)');
			}

		}

		context.tabbox(
			params.x, params.y, 
			params.w, params.h, 
			radius, gradient, false
		);

		context.setFontSize(this.fontSize);
		context.setFontType(this.fontType);

		if (this.hasFocus && this.canReceiveFocus && this.outlineOnFocus) {
			context.setColor("rgba(0, 0, 0, 1)");
			context.setShadow(0, 0, 3, "rgba(255, 255, 255, 0.4)");
			context.fillText(
				this.label, 
				params.x+textOffsetX, 
				params.y+textOffsetY
			);
			context.setShadow(0, 0, 0);
		}

		context.setText(
			this.label,
			params.x+textOffsetX+1,
			params.y+textOffsetY+1,
			this.color,
			"rgba(0, 0, 0, 0.4)"
		);

	}
});
/* -------------------------- */
/* Native (@) 2012 Stight.com */
/* -------------------------- */

Native.elements.export("UITab", {
	refresh : function(){
		var textWidth = Native.getTextWidth(
			this._label,
			this._fontSize,
			this._fontType
		);

		this._width = 14 + Math.round(textWidth) + 14;

		if (this.options.closable) {
			this._width += 22;
		}		
	},

	init : function(){
		var self = this,
			controller = self.parent,
			o = this.options;

		this.height = OptionalNumber(o.height, 24);
		this.fontSize = OptionalNumber(o.fontSize, 11);
		this.fontType = OptionalString(o.fontType, "arial");
		this.color = OptionalValue(o.color, "#aaaaaa");

		var textWidth = Native.getTextWidth(
			this.label,
			this.fontSize,
			this.fontType
		);

		this.width = 14 + Math.round(textWidth) + 14;

		if (o.closable) {
			this.width += 22;
		}		

		this.canReceiveFocus = true;

		this.addEventListener("mousedown", function(e){
			controller.selectTab(this.tabnum);
			e.stopPropagation();
		}, false);

		this.addEventListener("mouseover", function(e){
			this.hover = true;
		});

		this.addEventListener("mouseout", function(e){
			this.hover = false;
		});

		if (this.options.closable) {

			this.closeButton = this.add("UIButtonClose", {
				left : this.width - 26,
				top : (this.height-12)/2,
				width : 12,
				height : 12,
				color : 'rgba(0, 0, 0, 0.75)',
				background : "rgba(0, 0, 0, 0.3)"
			});

			this.closeButton.addEventListener("mouseup", function(){
				/*
				this.parent.g = {
					x : 0,
					y : this.parent.height/2
				};
				this.parent.bounceScale(0, 120, function(){
					this.parent._removeTab(this.tabnum);
				});
				*/

				controller._removeTab(self.tabnum);
				self.width = 0;
				self.height = 0;
				self.remove();

			}, false);

		}

		var __fireEvent = false,
			__startX = 0,
			__endX = 0,
			__currentDragingTabPosition = false;

		this.addEventListener("dragstart", function(e){
			__fireEvent = false;
			__startX = this.left;
			__endX = this.left + this.width;
			__currentDragingTabPosition = controller.getPosition(this.tabnum);
		}, false);

		this._root.addEventListener("dragover", function(e){
			if (__currentDragingTabPosition===false) { return false; }

			var i = __currentDragingTabPosition,
				curr = controller.getTab(i),
				next = controller.getTab(i+1),
				prev = controller.getTab(i-1);

			if (curr._absx + e.xrel < controller._absx) {
				curr.left = controller.left;
			} else if (curr._absx + curr.width + e.xrel > controller._absx + controller.width) {
				curr.left = controller.width - curr.width;
			} else {
				curr.left += e.xrel;
			}

			if (e.xrel>0) {
				if (next) {
					if (curr._absx+curr.width > (next._absx+next.width/2)) {
						controller.position = __currentDragingTabPosition+1;
						controller.fireEvent("tabswap", {
							tab : controller.selection,
							position : __currentDragingTabPosition+1
						});

						next.slideX(
							__startX, 
							180, 
							null, 
							Math.physics.cubicOut
						);
						
						controller.swapTabs(i, i+1);
						__fireEvent = true;

						__currentDragingTabPosition++;

						i = __currentDragingTabPosition;
						curr = controller.getTab(i);
						next = controller.getTab(i+1);
						prev = controller.getTab(i-1);

						__startX += prev.width - controller.overlap;
						__endX += prev.width - controller.overlap;

					}
				} 
			}

			if (e.xrel<0) {
				if (prev) {
					if (curr._absx < (prev._absx+prev.width/2) ) {
						controller.position = __currentDragingTabPosition-1;
						controller.fireEvent("tabswap", {
							tab : controller.selection,
							position : __currentDragingTabPosition-1
						});

						prev.slideX(
							__endX - prev.width, 
							180, 
							null, 
							Math.physics.cubicOut
						);

						controller.swapTabs(i, i-1);
						__fireEvent = true;

						__currentDragingTabPosition--;

						i = __currentDragingTabPosition;
						curr = controller.getTab(i);
						next = controller.getTab(i+1);
						prev = controller.getTab(i-1);

						__startX -= next.width - controller.overlap;
						__endX -= next.width - controller.overlap;

					}
				}
			}

		}, false);

		this.addEventListener("dragend", function(e){
			if (__currentDragingTabPosition===false) { return false; }
			
			let i = __currentDragingTabPosition,
				c = controller.getTab(i);

			c.slideX(__startX, 200, function(){});

			__currentDragingTabPosition = false;
			__startX = false;
			__endX = false;

			if (__fireEvent){
				controller.fireEvent("tabmove", {
					tab : this.tabnum,
					positions : controller.taborder
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
				context.setShadow(3, -2, 4, "rgba(0, 0, 0, 0.4)");
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
				gradient.addColorStop(0.25, 'rgba(255, 255, 255, 0.18)');
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
			params.y+textOffsetY,
			this.color,
			"rgba(0, 0, 0, 0.4)"
		);

	}
});
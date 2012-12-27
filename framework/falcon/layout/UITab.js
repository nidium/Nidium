/* -------------------------- */
/* Native (@) 2013 Stight.com */
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
			controller.selectTab(this.index);
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
		}

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
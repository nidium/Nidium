/* ------------------------+------------- */
/* Native Framework 2.0    | Falcon Build */
/* ------------------------+------------- */
/* (c) 2013 Stight.com - Vincent Fontaine */
/* -------------------------------------- */

Native.elements.export("UITab", {
	public : {
		label : {
			set : function(value){
				this.resizeElement();
				this.refreshTabController();
			}
		},

		fontSize : {
			set : function(value){
				this.resizeElement();
				this.refreshTabController();
			}
		},

		fontType : {
			set : function(value){
				this.resizeElement();
				this.refreshTabController();
			}
		},

		width : {
			set : function(value){
				this.refreshTabController();
			}
		}
	},

	init : function(){
		var self = this,
			o = this.options;

		this.canReceiveFocus = true;
		
		this.label = OptionalString(o.label, "Default");
		this.fontSize = OptionalNumber(o.fontSize, 11);
		this.fontType = OptionalString(o.fontType, "arial");
		this.height = OptionalNumber(o.height, 24);
		this.color = OptionalValue(o.color, "#aaaaaa");

		this.getState = function(){
			return this.selected;
		};

		this.setState = function(state){
			if (state === true){
				this.bringToFront();
			} else {
				this.sendToBack();
			}

			this.selected = state;
		};

		this.select = function(){
			this.setState(true);
		};

		this.unselect = function(){
			this.setState(false);
		};

		this.resizeElement = function(){
			var textWidth = Native.getTextWidth(
				this.label,
				this.fontSize,
				this.fontType
			);

			var width = 14 + Math.round(textWidth) + 14;

			if (this.options.closable) {
				width += 22;
				this.closeButton.left = width - 26;
			} else {
				width += 6;
			}
			this.width = width;
		};

		this.refreshTabController = function(){
			if (this.parent && this.parent.resetTabs){
				this.parent.resetTabs();
			}
		};

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

		this.resizeElement();
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
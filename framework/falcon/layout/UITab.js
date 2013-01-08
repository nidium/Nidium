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

		this.setProperties({
			canReceiveFocus	: true,
			label			: OptionalString(o.label, "Default"),
			fontSize  		: OptionalNumber(o.fontSize, 11),
			fontType  		: OptionalString(o.fontType, "arial"),

			paddingLeft		: OptionalNumber(o.paddingLeft, 16),
			paddingRight	: OptionalNumber(o.paddingLeft, 10),

			height 			: OptionalNumber(o.height, 24),
			color 			: OptionalValue(o.color, "#aaaaaa")
		});
		
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
			var width = DOMElement.draw.getInnerTextWidth(this);

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

		DOMElement.listeners.addHovers(this);

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
			radius = Math.max(3, this.radius);
			
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

		var gradient = DOMElement.draw.getSoftGradient(this, context, params);

		context.tabbox(
			params.x, params.y, 
			params.w, params.h, 
			radius, gradient, false
		);

		DOMElement.draw.label(this, context, params);
	}
});
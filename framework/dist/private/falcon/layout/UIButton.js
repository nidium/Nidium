/* ------------------------+------------- */
/* Native Framework 2.0    | Falcon Build */
/* ------------------------+------------- */
/* (c) 2013 nidium.com - Vincent Fontaine */
/* -------------------------------------- */

Native.elements.export("UIButton", {
	public : {
		label : {
			set : function(value){
				this.resizeElement();
			}
		},

		fontSize : {
			set : function(value){
				this.resizeElement();
			}
		},

		fontFamily : {
			set : function(value){
				this.resizeElement();
			}
		},

		paddingLeft : {
			set : function(value){
				this.resizeElement();
			}
		},
		
		paddingRight : {
			set : function(value){
				this.resizeElement();
			}
		}
	},

	init : function(){
		var o = this.options;

		this.setProperties({
			canReceiveFocus	: true,
			label			: OptionalString(o.label, "Button"),
			fontSize  		: OptionalNumber(o.fontSize, 11),
			fontFamily  	: OptionalString(o.fontFamily, "arial"),

			paddingLeft		: OptionalNumber(o.paddingLeft, 10),
			paddingRight	: OptionalNumber(o.paddingLeft, 10),

			height 			: OptionalNumber(o.height, 22),
			radius 			: OptionalNumber(o.radius, 2),
			background 		: OptionalValue(o.background, "#2277E0"),
			color 			: OptionalValue(o.color, "#ffffff"),
			cursor			: OptionalCursor(o.cursor, "pointer")
		});

		this.outlineColor = this.background;

		this.resizeElement = function(){
			this.width = DOMElement.draw.getInnerTextWidth(this);
		};

		this.resizeElement();

		DOMElement.listeners.addDefault(this);
	},

	draw : function(context){
		var	params = this.getDrawingBounds();

 		if (this.outlineColor && this.outline) {
			DOMElement.draw.outline(this);
		}

		if (__ENABLE_BUTTON_SHADOWS__) {
			if (this.selected){
				context.setShadow(0, 1, 0.75, "rgba(255, 255, 255, 0.08)");
			} else {
				context.setShadow(0, 2, 4, "rgba(0, 0, 0, 0.15)");
			}
		}

		DOMElement.draw.box(this, context, params);

		if (__ENABLE_BUTTON_SHADOWS__){
			context.setShadow(0, 0, 0);
		}

		if (this.selected){
			params.textOffsetY = 1;
		}

		DOMElement.draw.glassLayer(this, context, params);
		DOMElement.draw.label(this, context, params);
	}
});

/* ------------------------+------------- */
/* Native Framework 2.0    | Falcon Build */
/* ------------------------+------------- */
/* (c) 2013 nidium.com - Vincent Fontaine */
/* -------------------------------------- */

/* -------------------------------------------------------------------------- */
/* NSS PROPERTIES                                                             */
/* -------------------------------------------------------------------------- */

document.nss.add({
	"UIButton" : {
		label : "Button",
		color : "#ffffff",
		background : "#2277e0",

		radius : 2,
		height : 22,
		paddingLeft : 10,
		paddingRight : 10,

		fontSize : 11,
		textAlign : "center",

		autowidth : true,
		canReceiveFocus : true
	},

	"UIButton:hover" : {
		cursor : "pointer"
	},

	"UIButton:focus" : function(){
		this.outlineColor = this.inline.background;
	}
});

/* -------------------------------------------------------------------------- */
/* ELEMENT DEFINITION                                                         */
/* -------------------------------------------------------------------------- */

Native.elements.export("UIButton", {
	init : function(){
		var o = this.options;

		/* Element's Dynamic Properties */
		NDMElement.defineDynamicProperties(this, {
			autowidth : OptionalBoolean(o.autowidth, true)
		});

		/* Element's Static Properties */
		this.outlineColor = this.background;

		NDMElement.listeners.addDefault(this);
	},

	update : function(){
		if (this.autowidth) {
			this.width = NDMElement.draw.getInnerTextWidth(this);
		}
	},

	draw : function(context){
		var	params = this.getDrawingBounds();

 		if (this.outlineColor && this.outline) {
			NDMElement.draw.outline(this);
		}

		if (__ENABLE_BUTTON_SHADOWS__) {
			if (this.selected){
				context.setShadow(0, 1, 0.75, "rgba(255, 255, 255, 0.08)");
			} else {
				context.setShadow(0, 2, 4, "rgba(0, 0, 0, 0.15)");
			}
		}

		NDMElement.draw.box(this, context, params);

		if (__ENABLE_BUTTON_SHADOWS__){
			context.setShadow(0, 0, 0);
		}

		if (this.selected){
			params.textOffsetY = 1;
		}

		NDMElement.draw.glassLayer(this, context, params);
		NDMElement.draw.label(this, context, params);
	}
});

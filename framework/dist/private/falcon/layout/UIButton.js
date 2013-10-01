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
		textOffsetY : 0,

		shadowBlur : 4,
		shadowColor : "rgba(0, 0, 0, 0.15)",
		shadowOffsetX : 0,
		shadowOffsetY : 2,

		autowidth : true,
		canReceiveFocus : true
	},

	"UIButton:hover" : {
		cursor : "pointer"
	},

	"UIButton:selected" : {
		shadowBlur : 0.75,
		shadowColor : "rgba(255, 255, 255, 0.08)",
		shadowOffsetX : 0,
		shadowOffsetY : 1,
		textOffsetY : 1
	},

	"UIButton:hasFocus" : function(){
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

		this.addEventListener("contextmenu", function(e){
			e.preventDefault();
		}, false);

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

		NDMElement.draw.enableShadow(this);
		NDMElement.draw.box(this, context, params);
		NDMElement.draw.disableShadow(this);

		NDMElement.draw.glassLayer(this, context, params);
		NDMElement.draw.label(this, context, params);
	}
});

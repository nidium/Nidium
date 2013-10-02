/* ------------------------+------------- */
/* Native Framework 2.0    | Falcon Build */
/* ------------------------+------------- */
/* (c) 2013 nidium.com - Vincent Fontaine */
/* -------------------------------------- */

/* -------------------------------------------------------------------------- */
/* NSS PROPERTIES                                                             */
/* -------------------------------------------------------------------------- */

document.nss.add({
	"UITab" : {
		canReceiveFocus	: true,
		autowidth : true,
		closable : true,

		label : "Default",
		fontSize : 11,
		fontFamily : "arial",

		paddingLeft : 16,
		paddingRight : 10,

		shadowBlur : 4,
		shadowColor : "rgba(0, 0, 0, 0.25)",
		shadowOffsetX : 3,
		shadowOffsetY : -2,

		height : 24,
		background : "#262722",
		color : "#abacaa",
		cursor : "arrow"
	},

	"UITab:selected" : function(){
		this.shadowBlur = 2;
		this.shadowColor = this.inline.background || "#262722";
		this.shadowOffsetX = 0;
		this.shadowOffsetY = 2;
	}
});

/* -------------------------------------------------------------------------- */
/* ELEMENT DEFINITION                                                         */
/* -------------------------------------------------------------------------- */

Native.elements.export("UITab", {
	update : function(e){
		if (!e || e.property.in("width", "label", "fontSize", "fontFamily")) {
			if (this.autowidth) {
				var width = NDMElement.draw.getInnerTextWidth(this);

				if (this.options.closable) {
					width += 22;
					this.closeButton.left = width - 26;
				} else {
					width += 6;
				}
				this.width = width;
			}

			/* refresh TabController if any */
			if (this.parent && this.parent.resetTabs){
				if (!(this.parent._disableUpdate === true)) {
					this.parent.resetTabs();
				}
			}
		}
	},

	init : function(){
		var self = this,
			o = this.options;

		/* Element's Dynamic Properties */
		NDMElement.defineDynamicProperties(this, {
			autowidth : OptionalBoolean(o.autowidth, true),
			target : OptionalValue(o.target, null)
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

		NDMElement.listeners.addHovers(this);

		this.closeButton = this.add("UIButtonClose", {
			left : this.width - 26,
			top : (this.height-12)/2,
			width : 12,
			height : 12
		});

		if (!this.options.closable) {
			this.closeButton.hide();
		}
	},

	draw : function(context){
		var	params = this.getDrawingBounds(),
			radius = Math.max(3, this.radius);
			
		NDMElement.draw.enableShadow(this);

		context.tabbox(
			params.x, params.y, 
			params.w, params.h, 
			radius, this.background, false
		);

		NDMElement.draw.disableShadow(this);

		var gradient = NDMElement.draw.getSoftGradient(this, context, params);

		context.tabbox(
			params.x, params.y, 
			params.w, params.h, 
			radius, gradient, false
		);

		NDMElement.draw.label(this, context, params);
	}
});
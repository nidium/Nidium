/* ------------------------+------------- */
/* Native Framework 2.0    | Falcon Build */
/* ------------------------+------------- */
/* (c) 2013 nidium.com - Vincent Fontaine */
/* -------------------------------------- */

/* -------------------------------------------------------------------------- */
/* NSS PROPERTIES                                                             */
/* -------------------------------------------------------------------------- */

document.nss.add({
	"UICheckBox" : {
		canReceiveFocus	: true,
		label			: "",
		fontSize  		: 11,
		fontFamily  	: "menlo",
		textAlign 		: "left",

		textShadowOffsetX	: 1,
		textShadowOffsetY	: 1,
		textShadowBlur		: 1,
		textShadowColor 	: "rgba(0, 0, 0, 0.2)",

		shadowOffsetX	: 0,
		shadowOffsetY	: 2,
		shadowBlur		: 4,
		shadowColor 	: "rgba(0, 0, 0, 0.15)",

		paddingLeft 	: 2,
		paddingRight 	: 10,

		lineWidth  		: 1,
		autowidth 		: false,
		width 			: 200, 
		height 			: 22,
		radius 			: 2,
		background 		: "",
		color 			: "#ffffff",
		cursor 			: "arrow"
	},

	"UICheckBox:hover" : {
		cursor 			: "pointer"
	}
});

/* -------------------------------------------------------------------------- */
/* ELEMENT DEFINITION                                                         */
/* -------------------------------------------------------------------------- */

Native.elements.export("UICheckBox", {
	init : function(){
		var o = this.options;

		/* Element's Dynamic Properties */
		NDMElement.defineDynamicProperties(this, {
			autowidth : OptionalBoolean(o.autowidth, true)
		});

		this.name = OptionalString(o.name, "default");

		this.addEventListener("contextmenu", function(e){
			e.preventDefault();
		}, false);

		this.addEventListener("mousedown", function(e){
			this.selected = !this.selected;
			this.fireEvent("change", {
				value : this.value,
				state : this.selected
			});
		});

		this.select = function(){
			this.selected = true;
		};

		this.unselect = function(){
			this.selected = true;
		};
	},

	update : function(e){
		if (e.property.in(
			"width", "height",
			"label", "fontSize", "fontFamily",
			"paddingLeft", "paddingRight", "selected"
		)) {
			this.resize();
		}
	},

	resize : function(){
		if (this.autowidth) {
			this.width = NDMElement.draw.getInnerTextWidth(this) + this.height;
		}
	},

	draw : function(context){
		var	params = this.getDrawingBounds(),
			radius = this.height/2;

		context.lineWidth = this.lineWidth;

 		if (this.outlineColor && this.outline) {
			//NDMElement.draw.outline(this);
		}

		NDMElement.draw.enableShadow(this);
		NDMElement.draw.box(this, context, params);
		NDMElement.draw.disableShadow(this);

		var pad = 4;

		/* outer box */
		context.setShadow(0, 2, 4, "rgba(0, 0, 0, 0.15)");
			context.strokeStyle = "rgba(0, 0, 0, 0.15)";
			context.lineWidth = 0.75;

			NDMElement.draw.box(this, context, {
				x : params.x + pad,
				y : params.y + pad,
				w : params.h - 2*pad, // yes, params.h
				h : params.h - 2*pad
			}, "white", "rgba(0, 0, 0, 0.15)", null, this.radius-1);
		context.setShadow(0, 0, 0);

		var r = pad + Math.floor(this.height*0.10);

		context.lineWidth = 1;

		if (this.selected){
			var gradient = context.createLinearGradient(
				params.x+r, params.y+r,
				params.x+r+params.h-2*r, params.y+r+params.h-2*r
			);

			gradient.addColorStop(0.00, 'rgba(0, 0, 0, 0.070)');
			gradient.addColorStop(0.50, 'rgba(0, 0, 0, 0.080)');
			gradient.addColorStop(0.60, 'rgba(0, 0, 0, 0.090)');

			NDMElement.draw.box(this, context, {
				x : params.x + r,
				y : params.y + r,
				w : params.h - 2*r, // yes, params.h
				h : params.h - 2*r
			}, gradient, "rgba(0, 0, 0, 0.15)", 1, this.radius-1);

			var m = radius/1.3,
				x1 = params.x+m,
				y1 = params.y+m,
				x2 = params.x+params.h - m,
				y2 = params.y+params.h - m;

			context.strokeStyle = "rgba(0, 0, 0, 0.9)";
			context.lineWidth = 2*this.lineWidth;

			context.beginPath();
			context.moveTo(x1, y1);
			context.lineTo(x2, y2);
			context.stroke();

			context.beginPath();
			context.moveTo(x1, y2);
			context.lineTo(x2, y1);
			context.stroke();
		} else {
			NDMElement.draw.box(this, context, {
				x : params.x + r,
				y : params.y + r,
				w : params.h - 2*r, // yes, params.h
				h : params.h - 2*r
			}, "white", "rgba(0, 0, 0, 0.15)", 1, this.radius-1);
		}

		params.x += params.h + this.paddingLeft;

		NDMElement.draw.label(this, context, params);

	}
});

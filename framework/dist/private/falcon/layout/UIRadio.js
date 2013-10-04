/* ------------------------+------------- */
/* Native Framework 2.0    | Falcon Build */
/* ------------------------+------------- */
/* (c) 2013 nidium.com - Vincent Fontaine */
/* -------------------------------------- */

/* -------------------------------------------------------------------------- */
/* NSS PROPERTIES                                                             */
/* -------------------------------------------------------------------------- */

document.nss.add({
	"UIRadio" : {
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
		autowidth 		: true,
		width 			: 200, 
		height 			: 22,
		radius 			: 0,
		background 		: "",
		color 			: "#ffffff",
		cursor 			: "arrow"
	},

	"UIRadio:hover" : {
		cursor 			: "pointer"
	}
});

/* -------------------------------------------------------------------------- */
/* ELEMENT DEFINITION                                                         */
/* -------------------------------------------------------------------------- */

Native.elements.export("UIRadio", {
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
			this.selected = true;
		});
	},

	update : function(e){
		var key = e.property,
			val = e.value;

		if (key == "selected" && val === true) {
			this.__lock();
			document.layout.find("name", this.name).each(function(){
				this.selected = false;
			});

			this.selected = true;

			this.fireEvent("select", {
				value : this.value,
				state : true
			});
			this.__unlock();
		}

		if (key.in(
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
			radius = this.height/2.5;

		context.lineWidth = this.lineWidth;

 		if (this.outlineColor && this.outline) {
			//NDMElement.draw.outline(this);
		}

		NDMElement.draw.enableShadow(this);
		NDMElement.draw.box(this, context, params);
		NDMElement.draw.disableShadow(this);

		/* Outer Circle ----------------------------------------------------- */
		context.save();
		context.setShadow(0, 2, 4, "rgba(0, 0, 0, 0.15)");
			context.strokeStyle = "rgba(0, 0, 0, 0.15)";
			context.setColor("white");
			context.lineWidth = 0.75;

			context.beginPath();
			context.arc(
				params.x+radius+this.radius*0.25, 
				params.y+params.h*0.5, 
				radius, 0, 6.283185307179586, false
			);

			context.globalAlpha = 0.8;
			context.fill();

			context.globalAlpha = 0.6;
			context.stroke();
		context.setShadow(0, 0, 0);
		context.restore();


		/* Inner Gradient --------------------------------------------------- */
		var gradient = context.createRadialGradient(
			params.x+radius, params.y+radius, radius, 
			params.x+radius, params.y+radius, radius*0.1
		);

		gradient.addColorStop(0.00, 'rgba(255, 255, 255, 0.1)');
		gradient.addColorStop(1.00, 'rgba(255, 255, 255, 0.9)');

		context.save();
			context.lineWidth = 1;
			context.strokeStyle = "rgba(0, 0, 0, 0.10)";
			context.setColor(gradient);

			context.beginPath();
			context.arc(
				params.x+radius+this.radius*0.25, 
				params.y+params.h*0.5, 
				radius, 0, 6.283185307179586, false
			);
			context.fill();
			context.stroke();
		context.restore();
		/* Inner Gradient --------------------------------------------------- */

		var r = 5;
		context.lineWidth = 1;

		if (this.selected){
			context.setColor(this.color);
			context.strokeStyle = this.color;

			context.beginPath();
			context.arc(
				params.x+radius+this.radius*0.25,
				params.y+params.h*0.5, 
				radius-r, 0, 6.283185307179586, false
			);

			context.fill();

			context.setColor("rgba(0, 0, 0, 0.25)");
			context.strokeStyle = "rgba(0, 0, 0, 0.25)";
		} else {
			r = 5;
			context.setColor("rgba(0, 0, 0, 0.00)");
			context.strokeStyle = "rgba(0, 0, 0, 0.10)";
		}

		context.beginPath();
		context.arc(
			params.x+radius+this.radius*0.25,
			params.y+params.h*0.5, 
			radius-r, 0, 6.283185307179586, false
		);
		context.fill();
		context.stroke();

		params.x += params.h + this.paddingLeft;

		NDMElement.draw.label(this, context, params);

	}
});

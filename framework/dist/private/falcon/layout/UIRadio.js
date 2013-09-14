/* ------------------------+------------- */
/* Native Framework 2.0    | Falcon Build */
/* ------------------------+------------- */
/* (c) 2013 nidium.com - Vincent Fontaine */
/* -------------------------------------- */

Native.elements.export("UIRadio", {
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
		}
	},

	init : function(){
		var o = this.options;

		this.setProperties({
			canReceiveFocus	: true,
			label			: OptionalString(o.label, ""),
			fontSize  		: OptionalNumber(o.fontSize, 11),
			fontFamily  	: OptionalString(o.fontFamily, "arial"),
			textAlign 		: OptionalAlign(o.textAlign, "left"),

			textShadowOffsetX	: OptionalNumber(o.textShadowOffsetX, 1),
			textShadowOffsetY	: OptionalNumber(o.textShadowOffsetY, 1),
			textShadowBlur		: OptionalNumber(o.textShadowBlur, 1),
			textShadowColor 	: OptionalValue(
									o.textShadowColor,
									'rgba(0, 0, 0, 0.2)'
								),

			shadowOffsetX	: OptionalNumber(o.shadowOffsetX, 0),
			shadowOffsetY	: OptionalNumber(o.shadowOffsetY, 2),
			shadowBlur		: OptionalNumber(o.shadowBlur, 4),
			shadowColor 	: OptionalValue(
								o.shadowColor,
								'rgba(0, 0, 0, 0.15)'
							),

			name  			: OptionalString(o.name, "default"),
			paddingLeft 	: OptionalNumber(o.paddingLeft, 2),

			lineWidth  		: OptionalNumber(o.lineWidth, 1),
			height 			: OptionalNumber(o.height, 22),
			radius 			: OptionalNumber(o.radius, 0),
			background 		: OptionalValue(o.background, ""),
			color 			: OptionalValue(o.color, "#222222"),
			cursor 			: OptionalCursor(o.cursor, "pointer")
		});

		this.addEventListener("mousedown", function(e){
			Native.layout.find("name", this.name).each(function(){
				this.selected = false;
			});

			this.selected = true;
		});

		this.resizeElement = function(){
			this._innerTextWidth = DOMElement.draw.getInnerTextWidth(this);
		};

		this.resizeElement();

		this.width = OptionalNumber(o.width, this._innerTextWidth) + this.height + this.paddingLeft;

	},

	draw : function(context){
		var	params = this.getDrawingBounds(),
			radius = this.height/2.5;

		var gradient = context.createRadialGradient(
			params.x+radius, 
			params.y+radius, 
			radius, 
			params.x+radius, 
			params.y+radius*1.2, 
			radius/4
		);

		context.lineWidth = this.lineWidth;

		gradient.addColorStop(0.00, 'rgba(255, 255, 255, 0.4)');
		gradient.addColorStop(1.00, 'rgba(255, 255, 255, 0.9)');

 		if (this.outlineColor && this.outline) {
			DOMElement.draw.outline(this);
		}

		if (this.shadowBlur != 0) {
			context.setShadow(
				this.shadowOffsetX,
				this.shadowOffsetY,
				this.shadowBlur,
				this.shadowColor
			);
		}

		DOMElement.draw.box(this, context, params);
		context.setShadow(0, 0, 0);

		context.save();
		context.strokeStyle = this.color;

		context.beginPath();
		context.arc(
			params.x+radius+this.radius*0.25, 
			params.y+params.h*0.5, 
			radius, 0, 6.283185307179586, false
		);
		context.globalAlpha = 0.9;
		context.setColor(this.color);
		context.fill();
		context.globalAlpha = 0.2;
		context.stroke();
		context.restore();

		context.setShadow(0, 2, 3, "rgba(0, 0, 0, 0.15)");
		context.strokeStyle = "rgba(0, 0, 0, 0.3)";
		context.beginPath();
		context.arc(
			params.x+radius+this.radius*0.25, 
			params.y+params.h*0.5, 
			radius, 0, 6.283185307179586, false
		);
		context.setColor(gradient);
		context.fill();

		context.stroke();

		context.setShadow(0, 0, 0);

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

			context.setColor("rgba(20, 20, 20, 0.4)");
			context.strokeStyle = "rgba(0, 0, 0, 0.4)";
		} else {
			r = 3;
			context.setColor("rgba(10, 10, 10, 0.0)");
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

		DOMElement.draw.label(this, context, params);

	}
});

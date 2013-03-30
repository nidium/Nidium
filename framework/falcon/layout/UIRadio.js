/* ------------------------+------------- */
/* Native Framework 2.0    | Falcon Build */
/* ------------------------+------------- */
/* (c) 2013 Stight.com - Vincent Fontaine */
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

		fontType : {
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
			fontType  		: OptionalString(o.fontType, "arial"),
			textAlign 		: OptionalAlign(o.textAlign, "left"),

			textShadowOffsetX	: OptionalNumber(o.textShadowOffsetX, 1),
			textShadowOffsetY	: OptionalNumber(o.textShadowOffsetY, 1),
			textShadowBlur		: OptionalNumber(o.textShadowBlur, 1),
			textShadowColor 	: OptionalValue(
									o.textShadowColor,
									'rgba(0, 0, 0, 0.2)'
								),

			name  			: OptionalString(o.name, "default"),
			paddingLeft 	: OptionalNumber(o.paddingLeft, 4),

			height 			: OptionalNumber(o.height, 18),
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
			radius = this.height/2,

			textColor = "rgba(255, 255, 255, 0.8)",
			textShadow = "rgba(0, 0, 0, 0.15)";

		context.setFontSize(this.fontSize);

		var gradient = context.createRadialGradient(
			params.x+radius, 
			params.y+radius, 
			radius, 
			params.x+radius*0.7, 
			params.y+radius*0.7, 
			radius>>1
		);

		gradient.addColorStop(0.00,'#ccccff');
		gradient.addColorStop(1.00,'#ffffff');

		context.beginPath();

		context.arc(
			params.x+radius, 
			params.y+params.h*0.5, 
			radius, 0, 6.2831852, false
		);

		context.setColor(gradient);
		context.fill();
		context.lineWidth = 1;
		context.strokeStyle = "rgba(140, 140, 140, 0.7)";
		context.stroke();

		if (this.selected){
			var r = 4;
			context.beginPath();

			context.arc(
			params.x+radius, 
			params.y+params.h*0.5, 
			radius-r, 0, 6.2831852, false
			);

			context.setColor("rgba(60, 80, 200, 0.5)");
			context.fill();
			context.lineWidth = 1;
			context.strokeStyle = "rgba(0, 0, 180, 0.1)";
			context.stroke();
		}

		params.x += params.h + this.paddingLeft;

		DOMElement.draw.label(this, context, params);

	}
});

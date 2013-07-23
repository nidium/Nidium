/* ------------------------+------------- */
/* Native Framework 2.0    | Falcon Build */
/* ------------------------+------------- */
/* (c) 2013 Stight.com - Vincent Fontaine */
/* -------------------------------------- */

Native.elements.export("UITextField", {
	public : {
		value : {
			set : function(value){
				if (this.input.setText){
					this.input.setText(value);
					this.input.resizeElement();
					this.input.setStartPoint();
				}
			},

			get : function(){
				return this.input.text;
			}
		}
	},

	init : function(){
		var self = this;
			o = this.options;

		this.setProperties({
			fontSize  		: OptionalNumber(o.fontSize, 11),
			fontType  		: OptionalString(o.fontType, "arial"),

			multiline 		: OptionalBoolean(o.multiline, false),
			lineHeight 		: OptionalNumber(o.lineHeight, 22),

			width 			: OptionalNumber(o.width, 120),
			height 			: OptionalNumber(o.height, 22),

			background 		: OptionalValue(o.background, '#ffffff'),
			color 			: OptionalValue(o.color, "#000000"),

			radius 			: OptionalNumber(o.radius, 3),

			borderWidth 	: OptionalNumber(o.borderWidth, 1),
			borderColor 	: OptionalValue(o.borderColor, "rgba(0, 0, 0, 0.09)"),

			shadowBlur 		: OptionalNumber(o.shadowBlur, 4),
			shadowColor 	: OptionalValue(o.shadowColor, "rgba(0, 0, 0, 0.10)"),
			shadowOffsetY	: OptionalNumber(o.shadowOffsetY, 2),

			overflow		: true
		});

		this.view = this.add("UIView", {
			left : 0,
			top : 0,
			width : this.width,
			height : this.height,
			background : this.background,
			radius : this.radius,
			borderWidth : this.borderWidth,
			borderColor : this.borderColor,
			shadowBlur : this.shadowBlur,
			shadowColor : this.shadowColor,
			shadowOffsetY : this.shadowOffsetY,
			overflow : false,
			scrollable : true,
			scrollBarX : false,
			scrollBarY : false
		});

		this.input = this.view.add("UITextInput", {
			left : 3,
			top : 0,
			height : this.height - 4,
			fontSize : this.fontSize,
			fontType : this.fontType,
			lineHeight : this.lineHeight,
			text : this.value,
			textAlign : "left",
			editable : true,
			multiline : this.multiline
		});

		this.input.overlay.addEventListener("keydown", function(e){
			if (!self.input.hasFocus) return false;

			switch (e.keyCode) {
				case 13 : // enter
					self.fireEvent("submit", {
						value : self.input.text
					});
					break;
			};
		});

	},

	draw : function(context){}
});
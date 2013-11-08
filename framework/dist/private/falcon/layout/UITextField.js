/* ------------------------+------------- */
/* Native Framework 2.0    | Falcon Build */
/* ------------------------+------------- */
/* (c) 2013 nidium.com - Vincent Fontaine */
/* -------------------------------------- */

Native.elements.export("UITextField", {
	public : {
		value : {
			set : function(value){
				var text = OptionalString(value, "");
				this.input.setText(text);
				this.input.scrollToLineStart();
				this.input.resetUndo();
				if (text != "") this.input.hidePlaceHolder();
			},

			get : function(){
				return this.input.text;
			}
		},

		paddingLeft : { set : function(value){
			this.resizeElement(); }
		},

		paddingRight : { set : function(value){
			this.resizeElement(); }
		},

		paddingTop : { set : function(value){
			this.resizeElement(); }
		},

		paddingBottom : { set : function(value){
			this.resizeElement(); }
		},

		width : { set : function(value){
			this.resizeElement(); }
		},

		height : { set : function(value){
			this.resizeElement(); }
		},

		color : {
			set : function(value){
				this.input.color = value;
			}
		},

		editable : {
			set : function(value){
				this.input.editable = OptionalBoolean(value, true);
			},

			get : function(){
				return this.input.editable;
			}
		},

		placeholder : {
			set : function(value){
				this.input.placeholder = value;
			}
		}
	},

	init : function(){
		var self = this;
			o = this.options;

		this.setProperties({
			fontSize  		: OptionalNumber(o.fontSize, 12),
			fontFamily  	: OptionalString(o.fontFamily, "console"),

			multiline 		: OptionalBoolean(o.multiline, false),
			editable 		: OptionalBoolean(o.editable, true),

			lineHeight 		: OptionalNumber(o.lineHeight, 22),

			width 			: OptionalNumber(o.width, 120),
			height 			: OptionalNumber(o.height, 22),

			paddingLeft		: OptionalNumber(o.paddingLeft, 4),
			paddingRight	: OptionalNumber(o.paddingLeft, 4),
			paddingTop		: OptionalNumber(o.paddingTop, 3),
			paddingBottom	: OptionalNumber(o.paddingBottom, 3),

			background 		: OptionalValue(o.background, '#ffffff'),
			color 			: OptionalValue(o.color, "#000000"),

			radius 			: OptionalNumber(o.radius, 3),

			borderWidth 	: OptionalNumber(o.borderWidth, 1),
			borderColor 	: OptionalValue(o.borderColor, "rgba(0, 0, 0, 0.09)"),

			shadowBlur 		: OptionalNumber(o.shadowBlur, 4),
			shadowColor 	: OptionalValue(o.shadowColor, "rgba(0, 0, 0, 0.10)"),
			shadowOffsetY	: OptionalNumber(o.shadowOffsetY, 2),

			value  			: OptionalString(o.value, ""),
			placeholder 	: OptionalString(o.placeholder, ""),
			pattern  		: OptionalString(o.pattern, ""),

			overflow		: true
		});

		this.view = this.add("UIView", {
			left : this.paddingLeft,
			top : this.paddingTop,
			width : this.width - this.paddingLeft - this.paddingRight,
			height : this.height - this.paddingTop - this.paddingBottom,
			radius : this.radius,
			overflow : false,
			scrollable : true,
			scrollBarX : false,
			scrollBarY : false
		});

		this.input = this.view.add("UITextInput", {
			left : 0,
			top : 0,
			height : this.height - this.paddingTop - this.paddingBottom,
			fontSize : this.fontSize,
			fontFamily : this.fontFamily,
			lineHeight : this.lineHeight - this.paddingTop - this.paddingBottom,
			text : this.value,
			placeholder : this.placeholder,
			pattern : this.pattern,
			textAlign : "left",
			editable : this.editable,
			multiline : this.multiline
		});
		this.input.outlineHost = self;

		this.resizeElement = function(){
			this.view.left = this.paddingLeft;
			this.view.top = this.paddingTop;
			this.view.width = this.width - this.paddingLeft - this.paddingRight;
			this.view.height = this.height - this.paddingTop - this.paddingBottom;

			this.input.height = this.height - this.paddingTop - this.paddingBottom;
			this.input.lineHeight = this.lineHeight - this.paddingTop - this.paddingBottom;
			this.input.resizeElement();
		};

		this.input.addEventListener("focus", function(e){
			if (self.outlineOnFocus === false) return;
			self.outline = true;
			self.input.showOutline();
		}, false);

		this.input.addEventListener("blur", function(e){
			if (self.outlineOnFocus === false) return;
			self.outline = false;
		}, false);

		this.input.addEventListener("change", function(e){
			self.fireEvent("change", e);
		});

		this.submit = function(){
			if (self.input.pattern && self.input.match === false) {
				this.fireEvent("error", {
					value : self.input.text,
					pattern : self.input.pattern,
					error : {
						code : 3,
						message : "User input does not match defined pattern"
					}
				});
			} else {
				this.fireEvent("submit", {
					value : self.input.text,
					pattern : self.input.pattern,
					match : true
				});
			}
		};

		this.input.overlay.addEventListener("keydown", function(e){
			if (!self.input.hasFocus) return false;

			switch (e.keyCode) {
				case 13 : // enter
					self.submit();
					break;

				case 1073741912 : // Pad Enter
					self.submit();
					break;
			};
		});

		this.addEventListener("contextmenu", function(e){
			e.preventMouseEvents();
		});

	},

	draw : function(context){
		var	params = this.getDrawingBounds();

		if (this.outlineColor && this.outline) {
			NDMElement.draw.outline(this);
		}

		if (this.shadowBlur != 0) {
			context.setShadow(
				this.shadowOffsetX,
				this.shadowOffsetY,
				this.shadowBlur,
				this.shadowColor
			);
		}

		NDMElement.draw.box(this, context, params);
		context.setShadow(0, 0, 0);
	}
});
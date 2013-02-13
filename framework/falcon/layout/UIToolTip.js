/* ------------------------+------------- */
/* Native Framework 2.0    | Falcon Build */
/* ------------------------+------------- */
/* (c) 2013 Stight.com - Vincent Fontaine */
/* -------------------------------------- */

Native.elements.export("UIToolTip", {
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
		var self = this,
			o = this.options;

		this.setProperties({
			canReceiveFocus	: false,
			label			: OptionalString(o.label, ""),
			fontSize  		: OptionalNumber(o.fontSize, 11),
			fontType  		: OptionalString(o.fontType, "arial"),
			textAlign 		: OptionalAlign(o.textAlign, "left"),
			paddingLeft		: OptionalNumber(o.paddingLeft, 10),
			paddingRight	: OptionalNumber(o.paddingRight, 10),

			lineHeight		: OptionalNumber(o.lineHeight, 30),
			height			: OptionalNumber(o.height, 30),

			shadowBlur 		: OptionalNumber(o.shadowBlur, 4),
			shadowColor 	: OptionalValue(o.shadowColor, "rgba(0, 0, 0, 0.20"),
			shadowOffsetX	: OptionalNumber(o.shadowOffsetY, 0),
			shadowOffsetY	: OptionalNumber(o.shadowOffsetY, 2),

			textShadowOffsetX	: OptionalNumber(o.textShadowOffsetX, 1),
			textShadowOffsetY	: OptionalNumber(o.textShadowOffsetY, 1),
			textShadowBlur		: OptionalNumber(o.textShadowBlur, 1),
			textShadowColor 	: OptionalValue(
									o.textShadowColor,
									'rgba(0, 0, 0, 0.2)'
								),

			radius 			: OptionalNumber(o.radius, 3),
			background 		: OptionalValue(o.background, "#ffffff"),
			border 			: OptionalValue(o.border, "rgba(0, 0, 0, 0.1)"),
			color 			: OptionalValue(o.color, "#444444")
		});

		if (this.parent){
			this.left = this.parent.width + 18;
			this.top = (this.parent.height - this.height) / 2;

			this.parent.hoverize(
				function(e){
					if (self.enabled) self.open();
				},
				function(e){
					if (self.enabled) self.close();
				}
			);
		}

		this.resizeElement = function(){
			this._innerTextWidth = DOMElement.draw.getInnerTextWidth(this);
		};

		this.enable = function(){
			this.enabled = true;
		};

		this.disable = function(){
			this.enabled = false;
			this.close();
		};

		this.open = function(){
			if (this.opened) return false;

			var parent = this.parent,
				duration = 80;

			if (self.hinting) return false;
			self.hinting = true;

			self.show();

			if (self.unhinting) {
				self.unhinting = false;
				self.destroyCurrentAnimations("opacity");
				self.destroyCurrentAnimations("left");
				duration = 80;
			} else {
				self.left = parent.width;
				self.opacity = 0;
				duration = 200;
			}

			self.fadeIn(duration, function(){
				self.hinting = false;
				self.closed = false;
				self.opened = true;
			});

			self.slideX(
				parent.width+18, 
				duration, 
				null, 
				Math.physics.quadOut
			);
		};

		this.close = function(){
			if (this.closed) return false;

			var parent = this.parent;

			if (self.hinting) {
				self.destroyCurrentAnimations("opacity");
				self.destroyCurrentAnimations("left");
				self.hide();
				self.hinting = false;
				self.unhinting = false;
			} else {
				self.unhinting = true;
				self.fadeOut(200, function(){
					self.hinting = false;
					self.unhinting = false;
					self.closed = true;
					self.opened = false;
					self.hide();
				});
				
				self.slideX(
					parent.width, 
					200, 
					null, 
					Math.physics.quadOut
				);
			}
		};

		this.resizeElement();

		this.width = OptionalNumber(o.width, this._innerTextWidth);
		this.hide();
	},

	draw : function(context){
		var	params = this.getDrawingBounds();

		context.setShadow(
			this.shadowOffsetX,
			this.shadowOffsetY,
			this.shadowBlur,
			this.shadowColor
		);

		var ox = params.x-2,
			oy = params.y+7,
			w = 16,
			h = 16,
			cx = ox + w/2,
			cy = oy + h/2,
			rad = 45 * (Math.PI/180);

		context.save();
		context.translate(cx, cy);
		context.rotate(rad);
		context.translate(-cx, -cy);

		context.roundbox(
			ox, oy, 
			w, h,
			0,
			this.background,
			this.border
		);
		context.restore();

		DOMElement.draw.box(this, context, params);
		context.setShadow(0, 0, 0);

		context.save();
		context.translate(cx, cy);
		context.rotate(rad);
		context.translate(-cx, -cy);

		context.roundbox(
			ox, oy, 
			w, h,
			0,
			this.background,
			null
		);
		context.restore();

		DOMElement.draw.label(this, context, params);


	}
});
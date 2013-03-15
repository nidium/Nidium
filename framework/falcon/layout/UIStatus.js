/* ------------------------+------------- */
/* Native Framework 2.0    | Falcon Build */
/* ------------------------+------------- */
/* (c) 2013 Stight.com - Vincent Fontaine */
/* -------------------------------------- */

Native.elements.export("UIStatus", {
	init : function(){
		var o = this.options;

		this.setProperties({
			canReceiveFocus	: false,
			label			: OptionalString(o.label, "Ready"),
			fontSize  		: OptionalNumber(o.fontSize, 10),
			fontType  		: OptionalString(o.fontType, "arial"),
			textAlign 		: OptionalAlign(o.textAlign, "left"),

			progressBarColor : OptionalValue(o.progressBarColor, false),
			progressBarLeft : OptionalNumber(o.progressBarLeft, 40),
			progressBarRight : OptionalNumber(o.progressBarRight, 40),

			textShadowOffsetX	: OptionalNumber(o.textShadowOffsetX, 1),
			textShadowOffsetY	: OptionalNumber(o.textShadowOffsetY, 1),
			textShadowBlur		: OptionalNumber(o.textShadowBlur, 1),
			textShadowColor 	: OptionalValue(
									o.textShadowColor,
									'rgba(0, 0, 0, 0.25)'
								),

			value 			: OptionalNumber(o.value, 0),
			paddingLeft		: OptionalNumber(o.paddingLeft, 5),
			height 			: OptionalNumber(o.height, 20),
			radius 			: OptionalNumber(o.radius, 0),
			background 		: OptionalValue(o.background, "rgba(38, 39, 34, 0.85)"),
			color 			: OptionalValue(o.color, "#eeeeee"),
			opacity			: 1.0
		});

		this.spinner = new UISpinner(this, {
			height : this.height-6,
			width : this.height-6,
			left : this.width-(this.height-6)-3,
			top : 3,

			dashes : 10,
			color : this.color,
			speed : 20,
			opacity : 0.7,
			radius : 2
		});

		this.open = function(duration=600){
			if (this.closed === false || this.opening) return false;
			this.visible = true;
			this.spinner.opacity = 0.7;
			this.spinner.play();
			this.fadeIn(80);
			
			this.opening = true;
			this.animate(
				"top",
				this.parent.height,
				this.parent.height - this.height,
				duration,
				function(){
					this.opening = false;
					this.visible = true;
					this.closed = false;
				},
				Math.physics.quintOut
			);
			return this;
		},

		this.close = function(duration=200){
			var that = this;
			if (this.closed || this.closing) return false;
			this.visible = true;
			this.closing = true;			
			this.spinner.fadeOut(400, function(){
				this.stop();

				that.animate(
					"top",
					that.parent.height - that.height,
					that.parent.height,
					duration,
					function(){
						that.closing = false;
						that.visible = false;
						that.closed = true;
					},
					Math.physics.quintIn
				);

				that.fadeOut(duration*1.2);

			});
			return this;
		},

		this.position = "fixed";
		this.width = this.parent.width;
		this.top = this.parent.height;

		this.opacity = 0;
		this.closed = true;
		this.visible = false;
		this.spinner.stop();
		this.spinner.opacity = 0;
	},

	draw : function(context){
		var	params = this.getDrawingBounds();

		DOMElement.draw.box(this, context, params);
		DOMElement.draw.label(this, context, params);

		var gradient = DOMElement.draw.getSmoothGradient(this, context, params);
		DOMElement.draw.box(this, context, params, gradient);

		if (this.progressBarColor && this.value!=0) {
			var mw = params.w - this.progressBarRight  - this.progressBarLeft,
				mh = 2.5,
				mx = this.progressBarLeft,
				my = params.h/2;

			var ga = context.globalAlpha,
				v = this.value.bound(0, 100),
				pixelValue = Math.round(mw*v/100);

			context.lineWidth = 1.5;
			context.strokeStyle = "rgba(0, 0, 0, 0.40)";
			context.beginPath();
			context.moveTo(mx, my);
			context.lineTo(mx+mw, my);
			context.stroke();

			context.lineWidth = 0.25;
			context.strokeStyle = "rgba(255, 255, 255, 0.6)";
			context.beginPath();
			context.moveTo(mx, my+1);
			context.lineTo(mx+mw, my+1);
			context.stroke();

			context.globalAlpha = 0.85;
			context.setShadow(0, 0, 4, this.progressBarColor);
			context.globalAlpha = ga;
			context.roundbox(
				mx, my-0.5, 
				pixelValue, mh,
				3, this.progressBarColor, false
			);
			context.setShadow(0, 0, 0);
		}

		context.lineWidth = 1;
		context.strokeStyle = "rgba(0, 0, 0, 1.0)";
		context.beginPath();
		context.moveTo(params.x, params.y);
		context.lineTo(params.x+params.w, params.y);
		context.stroke();

		context.lineWidth = 0.25;
		context.strokeStyle = "rgba(255, 255, 255, 0.8)";
		context.beginPath();
		context.moveTo(params.x, params.y+0.5);
		context.lineTo(params.x+params.w, params.y+0.5);
		context.stroke();
	}
});

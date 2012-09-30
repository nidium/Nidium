/* -------------------------- */
/* Native (@) 2012 Stight.com */
/* -------------------------- */

UIElement.extend("UIButton", {
	init : function(){
		canvas.setFontSize(this.fontSize);
		this.w = 10 + Math.round(canvas.measureText(this.label)) + 10;
		this.h = 22;
		this.flags._canReceiveFocus = true;

		this.addEventListener("mousedown", function(e){
			this.selected = true;
		});

		this.addEventListener("mouseup", function(e){
			this.selected = false;
		});

		this.addEventListener("dragend", function(e){
			this.selected = false;
		});

		this.addEventListener("mouseover", function(e){
			this.hover = true;
		});

		this.addEventListener("mouseout", function(e){
			this.hover = false;
		});

	},

	draw : function(){
		var params = {
				x : this._x,
				y : this._y,
				w : this.w,
				h : this.h
			},

			radius = Math.max(3, this.radius),
			label = this.label,

			ctx = canvas;

		ctx.setFontSize(this.fontSize);
		this.w = 10 + Math.round(ctx.measureText(this.label)) + 10;
		this.h = 22;

		var	textWidth = Math.round(ctx.measureText(label)),
			textHeight = 10,
			w = params.w,
			h = params.h,
			textOffsetX = 9,
			textOffsetY = (h-textHeight)/2 + 9,
			textColor = '#e0e0e0',
			textShadow = '#000000';

		this.shadow = true;
		if (this.shadow) {
			if (this.selected){
				ctx.setShadow(0, 2, 1, "rgba(255, 255, 255, 0.05)");
			} else {
				ctx.setShadow(0, 2, 3, "rgba(0, 0, 0, 0.5)");
			}
		}
		ctx.roundbox(params.x, params.y, w, h, radius, this.background, false);
		if (this.shadow){
			ctx.setShadow(0, 0, 0);
		}

		var gdBackground = ctx.createLinearGradient(params.x, params.y, params.x, params.y+params.h);

		if (this.selected){

			textOffsetY++;
			textColor = "rgba(255, 255, 255, 0.8)";
			textShadow = "rgba(255, 255, 255, 0.15)";

			gdBackground.addColorStop(0.00, 'rgba(0, 0, 0, 0.8)');
			gdBackground.addColorStop(0.25, 'rgba(0, 0, 0, 0.6)');
			gdBackground.addColorStop(1.00, 'rgba(0, 0, 0, 0.6)');

		} else {

			textColor = "rgba(255, 255, 255, 0.95)";
			textShadow = "rgba(0, 0, 0, 0.2)";

			if (this.hover){
				gdBackground.addColorStop(0.00, 'rgba(255, 255, 255, 0.7)');
				gdBackground.addColorStop(0.50, 'rgba(255, 255, 255, 0.3)');
				gdBackground.addColorStop(0.50, 'rgba(255, 255, 255, 0.1)');
				gdBackground.addColorStop(0.80, 'rgba(0, 0, 0, 0.1)');
				gdBackground.addColorStop(1.00, 'rgba(0, 0, 0, 0.3)');
			} else {
				gdBackground.addColorStop(0.00, 'rgba(255, 255, 255, 0.4)');
				gdBackground.addColorStop(0.50, 'rgba(255, 255, 255, 0.1)');
				gdBackground.addColorStop(0.50, 'rgba(255, 255, 255, 0.0)');
				gdBackground.addColorStop(0.50, 'rgba(0, 0, 0, 0.0)');
				gdBackground.addColorStop(0.80, 'rgba(0, 0, 0, 0.1)');
				gdBackground.addColorStop(1.00, 'rgba(0, 0, 0, 0.3)');
			}

		}

		ctx.roundbox(params.x, params.y, w, h, radius, gdBackground, false);

		ctx.setFontSize(this.fontSize);

	//	ctx.setColor(textShadow);
	//	ctx.fillText(label, params.x+textOffsetX+1, params.y+textOffsetY+1);

	//	if (this.shadow) { ctx.setShadow(1, 1, 1, '#000000'); }
		ctx.setColor(textColor);
		ctx.fillText(label, params.x+textOffsetX, params.y+textOffsetY);
	//	if (this.shadow){ ctx.setShadow(0, 0, 0); }


	}
});

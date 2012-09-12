/* -------------------------- */
/* Native (@) 2012 Stight.com */
/* -------------------------- */

UIElement.extend("UITextInput", {
	init : function(){
		var self = this;

		this.w = 140;
		this.h = 24;
		this.background = this.options.background || "#191a18";
		this.color = this.options.color || "#ffffff";

		this.textView = this.add("UIText", {
			x : 2,
			y : 2, 
			w : self.w - 4,
			h : self.h - 4,
			fontSize : 11,
			text : self.text,
			background : "rgba(255, 255, 255, 0.85)"
		});

	},

	draw : function(){
		var params = {
				x : this._x,
				y : this._y,
				w : this.w,
				h : this.h
			},
	
			radius = Math.max(4, this.radius),
			textHeight = 10,
			textOffsetX = 8,
			textOffsetY = (this.h-textHeight)/2 + 9,
			textColor = this.color,
			textShadow = '#000000';


		this.shadow = true;
		if (this.shadow) {
			if (this.selected){
				canvas.setShadow(0, 2, 1, "rgba(255, 255, 255, 0.05)");
			} else {
				canvas.setShadow(0, 2, 3, "rgba(0, 0, 0, 0.5)");
			}
		}
		canvas.roundbox(params.x, params.y, params.w, params.h, radius, this.background, false); // main view
		if (this.shadow){
			canvas.setShadow(0, 0, 0);
		}

		var gdBackground = canvas.createLinearGradient(params.x, params.y, params.x, params.y+params.h);
		gdBackground.addColorStop(0.00, 'rgba(255, 255, 255, 0.20)');
		gdBackground.addColorStop(0.10, 'rgba(255, 255, 255, 0.05)');
		gdBackground.addColorStop(0.90, 'rgba(255, 255, 255, 0.00)');

		canvas.roundbox(params.x, params.y, params.w, params.h, radius, gdBackground, false); // main view
	}
});
/* -------------------------- */
/* Native (@) 2012 Stight.com */
/* -------------------------- */

UIElement.extend("UIWindow", {
	init : function(){
		var self = this;

		this.background = this.options.background || "#191a18";
		this.color = this.options.color || "#ffffff";
		this.name = this.options.name || "Default";

		this.handleBar = this.createElement("UIView", {
			x : 0,
			y : 0,
			w : self.w,
			h : 24,
			radius : 4,
			background : "rgba(0, 0, 0, 0.75)",
			color : "#888888"
		});

		this.handleBar.addEventListener("drag", function(e){
			let c = this,
				p = this.parent.parent;

			c.left += e.xrel;
		});


		this.handleBar.closeButton = this.createElement("UIButtonClose", {
			x : this.w-18,
			y : 4,
			w : 16,
			h : 16,
			background : "rgba(0, 0, 0, 0.75)",
			color : "#888888"
		});

		this.content = this.createElement("UIView", {
			x : 3,
			y : 24,
			w : self.w-6,
			h : self.h-27,
			radius : 4,
			background : "#ffffff",
			color : "#333333"
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
			textOffsetY = (24-textHeight)/2 + 9,
			textColor = this.color,
			textShadow = '#000000',

			label = this.name;



		this.shadow = true;
		if (this.shadow) {
			canvas.setShadow(2, 2, 12, "rgba(0, 0, 0, 0.5)");
		}

		canvas.roundbox(params.x, params.y, params.w, params.h, radius, this.background, false); // main view

		if (this.shadow){
			canvas.setShadow(0, 0, 0);
		}

		var gdBackground = canvas.createLinearGradient(params.x, params.y, params.x, params.y+24);
		gdBackground.addColorStop(0.00, 'rgba(255, 255, 255, 0.20)');
		gdBackground.addColorStop(0.10, 'rgba(255, 255, 255, 0.05)');
		gdBackground.addColorStop(0.90, 'rgba(255, 255, 255, 0.00)');

		canvas.roundbox(params.x, params.y, params.w, params.h, radius, gdBackground, false); // main view

		canvas.fontSize = 11;
		canvas.fillStyle = textShadow;
		canvas.fillText(label, params.x+textOffsetX+1, params.y+textOffsetY+1);

		canvas.fillStyle = textColor;
		canvas.fillText(label, params.x+textOffsetX, params.y+textOffsetY);

	}
});
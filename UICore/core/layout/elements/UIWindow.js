/* -------------------------- */
/* Native (@) 2012 Stight.com */
/* -------------------------- */

UIElement.extend("UIWindow", {
	init : function(){
		var self = this;

		this.flags._canReceiveFocus = true;
		this.background = this.options.background || "#191a18";
		this.color = this.options.color || "#ffffff";
		this.name = this.options.name || "Default";
		this.shadowBlur = this.options.shadowBlur || 12;
		this.shadowColor = this.options.shadowColor || "rgba(0, 0, 0, 0.5)";

		this.addEventListener("mousedown", function(e){
			this.bringToTop();
			e.stopPropagation();
		}, false);

		this.addEventListener("mouseover", function(e){
			e.stopPropagation();
		}, false);

		this.handle = this.add("UIView", {
			x : 0,
			y : 0,
			w : self.w,
			h : 24,
			radius : 4,
			background : "rgba(0, 0, 0, 0.05)",
			color : "#888888"
		});


		if (this.options.movable) {
			this.handle.addEventListener("dragstart", function(){
				self.bounceScale(1.1, 80, function(){

				});
				self.shadowBlur = 30;
				self.shadowColor = "rgba(0, 0, 0, 0.95)";
			}, false);

			this.handle.addEventListener("drag", function(e){
				this.parent.left += e.xrel;
				this.parent.top += e.yrel;
			});

			this.handle.addEventListener("dragend", function(){
				self.bounceScale(1, 50, function(){
					
				});
				self.shadowBlur = this.options.shadowBlur || 12;
				self.shadowColor = this.options.shadowColor || "rgba(0, 0, 0, 0.5)";
			}, false);

		}

		if (this.options.closeable) {
			this.handle.closeButton = this.add("UIButtonClose", {
				x : this.w-18,
				y : 4,
				w : 16,
				h : 16,
				background : "rgba(0, 0, 0, 0.75)",
				color : "#888888"
			});

			this.handle.closeButton.addEventListener("mousedown", function(){
				self.bounceScale(0, 120, function(){
				});
				self.shadowBlur = 6;
				self.shadowColor = "rgba(0, 0, 0, 0.20)";
				e.stopPropagation();
			}, false);
		}

		this.contentView = this.add("UIView", {
			x : 3,
			y : 24,
			w : self.w-6,
			h : self.h-27,
			radius : 4,
			background : "#ffffff",
			color : "#333333"
		});

		if (this.options.resizable) {
			this.resizer = this.add("UIWindowResizer");
		}

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
			canvas.setShadow(0, 15, this.shadowBlur, this.shadowColor);
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
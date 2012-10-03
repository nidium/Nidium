/* -------------------------- */
/* Native (@) 2012 Stight.com */
/* -------------------------- */

UIElement.extend("UIWindow", {
	init : function(){
		var self = this;

		this.flags._canReceiveFocus = true;
		this.color = OptionalValue(this.options.color, "#ffffff");
		this.name = OptionalString(this.options.name, "Default");
		this.shadowBlur = OptionalNumber(this.options.shadowBlur, 12);
		this.shadowColor = OptionalValue(this.options.shadowColor, "rgba(0, 0, 0, 0.5)");

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
			color : "#888888",
			callback : function(){
				var p = this.parent,
					textHeight = 10,
					textOffsetX = 8,
					textOffsetY = (24-textHeight)/2 + 9;

				canvas.setFontSize(11);
				canvas.setColor(p.color);
				canvas.fillText(p.name, p._x+textOffsetX, p._y+textOffsetY);
			}
		});

		if (this.options.movable) {
			this.handle.addEventListener("dragstart", function(){
				self.set("scale", 1.1, 80);
				self.set("blur", 1, 80);
				self.set("shadowBlur", 20, 70);
				self.shadowColor = "rgba(0, 0, 0, 0.95)";
			}, false);

			this.handle.addEventListener("drag", function(e){
				this.parent.left += e.xrel;
				this.parent.top += e.yrel;
			});

			this.handle.addEventListener("dragend", function(){
				self.set("scale", 1, 50);
				self.set("blur", 0, 50);
				self.set("shadowBlur", self.options.shadowBlur || 12, 50);
				self.shadowColor = self.options.shadowColor || "rgba(0, 0, 0, 0.5)";
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

			this.handle.closeButton.addEventListener("mousedown", function(e){
				self.set("scale", 0, 120, function(){});
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
	
			radius = Math.max(4, this.radius);

		if (this.blur){
			this.blurbox = {
				x : this.__x,
				y : this.__y,
				w : this.__w,
				h : 24*this._scale
			};
		}

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
	}
});
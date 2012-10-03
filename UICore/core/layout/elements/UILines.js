/* -------------------------- */
/* Native (@) 2012 Stight.com */
/* -------------------------- */

UIElement.extend("UILine", {

	__construct : function(){

	},

	init : function(){
		var self = this;

		this.x1 = OptionalNumber(this.options.x1, 0);
		this.y1 = OptionalNumber(this.options.y1, 0);
		this.x2 = OptionalNumber(this.options.x2, 0);
		this.y2 = OptionalNumber(this.options.y2, 0);

		this.lineWidth = OptionalNumber(this.options.lineWidth, 1);

		/*
		this.addEventListener("drag", function(e){
			var dx = e.xrel / this._scale,
				dy = e.yrel / this._scale;

				this._x += dx;
				this._y += dy;
				this.x += dx;
				this.y += dy;

				self.controlPoint3.x += dx
				self.controlPoint3.y += dy;
		});
		*/

		this.updateParameters = function(){
			this.x1 = this.controlPoint1.x;
			this.y1 = this.controlPoint1.y;
			this.x2 = this.controlPoint2.x;
			this.y2 = this.controlPoint2.y;

			this.w = Math.abs(this.x2 - this.x1);
			this.h = Math.abs(this.y2 - this.y1);

			this.g = {
				x : this.x1>=this.x2 ? this.x2 : this.x1,
				y : this.y1>=this.y2 ? this.y2 : this.y1
			};
		};

		this.controlPoint1 = this.add("UIControlPoint", {
			x : self.x1,
			y : self.y1,
			color : self.color
		});

		this.controlPoint2 = this.add("UIControlPoint", {
			x : self.x2,
			y : self.y2,
			color : self.color
		});

		this.addEventListener("change", function(e){
			this.updateParameters();
		}, false);

		if (this.options.split){
			this.control = {
				x : this.x1 + (this.x2 - this.x1)/2,
				y : this.y1 + (this.y2 - this.y1)/2
			};

			this.controlPoint3 = this.add("UIControlPoint", {
				x : self.control.x,
				y : self.control.y,
				color : self.color
			});

			this.controlPoint3.addEventListener("drag", function(e){
				self.control.x += e.dx;
				self.control.y += e.dy;
			}, false);

		}

	},

	isPointInside : function(mx, my){
		var x1 = this.__x + this.x1*this._scale,
			y1 = this.__y + this.y1*this._scale,
			x2 = x1 + this.__w,
			y2 = y1 + this.__h;

		return	(mx>=x1 && mx<=x2 && my>=y1 && my<=y2) ? true : false;
	},

	draw : function(){
		this.updateParameters();

		var params = {
				x : this._x,
				y : this._y,
				w : this.w,
				h : this.h,
			};
		
		/*
		canvas.setColor("rgba(0, 0, 0, 0.5)");
		var r = {
			x : this._x + this.x1,
			y : this._y + this.y1,
			w : this.w,
			h : this.h
		}
		canvas.fillRect(r.x, r.y, r.w, r.h);
		*/

		canvas.strokeStyle = this.color;
		canvas.lineWidth = this.lineWidth;
		
		canvas.beginPath();
		canvas.moveTo(params.x + this.x1, params.y + this.y1);
		
		if (this.options.split){
			
			if (this.options.split=="quadratic"){
				canvas.quadraticCurveTo(
					params.x + this.control.x + this.x, params.y + this.control.y + this.y, 
					params.x + this.x2, params.y + this.y2
				);
			} else {
				canvas.lineTo(params.x + this.control.x + this.x, params.y + this.control.y + this.y);
				canvas.lineTo(params.x + this.x2, params.y + this.y2);
			}

		} else {
			canvas.lineTo(params.x + this.x2, params.y + this.y2);
		}

		canvas.stroke();

	}
});

UIElement.extend("UIControlPoint", {
	init : function(){
		var self = this;
		this.w = 16;
		this.h = 16;

		this.radius = OptionalNumber(this.options.radius, 3);
		this.background = OptionalValue(this.options.background, "rgba(0, 0, 0, 0.5)"),
		this.lineWidth = 1;
		this.opacity = 0.5;

		this.g = {
			x : - this.w/2,
			y : - this.h/2
		};

		this.addEventListener("mousedown", function(e){
			this.opacity = 1.00;
			e.stopPropagation();
		}, true);

		this.addEventListener("mouseup", function(e){
			this.set("opacity", 0.50, 90);
			e.stopPropagation();
		}, true);

		this.addEventListener("dragstart", function(e){
			this.opacity = 1.00;
		}, false);

		this.addEventListener("drag", function(e){
			this.left += e.dx; // e.dx = e.xrel / this._scale
			this.top += e.dy; // e.dy = e.yrel / this._scale
			this.parent.fireEvent("change", e);
			this.opacity = 1.00;
			e.stopPropagation();
		}, false);

		this.addEventListener("dragend", function(e){
			this.set("opacity", 0.50, 90);
		});

	},

	isPointInside : function(mx, my){
		var x1 = this.__x - this.__w/2,
			y1 = this.__y - this.__h/2,
			x2 = x1 + this.__w,
			y2 = y1 + this.__h;

		return	(mx>=x1 && mx<=x2 && my>=y1 && my<=y2) ? true : false;
	},

	draw : function(){
		var hx = this.w / 2,
			hy = this.h / 2,
			params = {
				x : this._x,
				y : this._y,
				w : this.w,
				h : this.h
			};

		canvas.roundbox(params.x-hx+2, params.y-hy+2, params.w-4, params.h-4, this.radius, '', "#000000", this.lineWidth); // main view
		canvas.roundbox(params.x-hx+4, params.y-hy+4, params.w-8, params.h-8, 0, this.background, "#ffffff", this.lineWidth); // main view

	}
});



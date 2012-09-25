/* -------------------------- */
/* Native (@) 2012 Stight.com */
/* -------------------------- */

UIElement.extend("UILine", {

	__construct : function(){
		this.x1 = this.options.x1 || 0;
		this.y1 = this.options.y1 || 0;
		this.x2 = this.options.x2 || 0;
		this.y2 = this.options.y2 || 0;

		this.w = Math.abs(this.x2 - this.x1);
		this.h = Math.abs(this.y2 - this.y1);

		this.g = {
			x : this.x1>=this.x2 ? this.x2 : this.x1,
			y : this.y1>=this.y2 ? this.y2 : this.y1
		};

	},

	init : function(){
		var self = this;

		this.x1 = this.options.x1 || 0;
		this.y1 = this.options.y1 || 0;
		this.x2 = this.options.x2 || 0;
		this.y2 = this.options.y2 || 0;

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

		this.controlPoint1 = this.add("UIControlPoint", {
			x : this.x1,
			y : this.y1,
			color : this.color
		});

		this.controlPoint2 = this.add("UIControlPoint", {
			x : this.x2,
			y : this.y2,
			color : this.color
		});

		this.controlPoint1.opacity = 0.1;
		this.controlPoint2.opacity = 0.1;

		this.controlPoint1.addEventListener("mouseover", function(e){
			this.opacity = 0.6;
		});
		this.controlPoint1.addEventListener("mouseout", function(e){
			this.opacity = 0.1;
		});
		this.controlPoint2.addEventListener("mouseover", function(e){
			this.opacity = 0.6;
		});
		this.controlPoint2.addEventListener("mouseout", function(e){
			this.opacity = 0.1;
		});

		if (this.options.split){
			this.control = {
				x : this.x1 + (this.x2 - this.x1)/2,
				y : this.y1 + (this.y2 - this.y1)/2
			};

			this.controlPoint3 = this.add("UIControlPoint", {
				x : this.control.x,
				y : this.control.y,
				color : this.color
			});

			this.controlPoint3.opacity = 0.1;
			
			this.controlPoint3.addEventListener("drag", function(e){
				var dx = e.xrel / self._scale,
					dy = e.yrel / self._scale;

				this._x += dx;
				this._y += dy;
				this.x += dx;
				this.y += dy;

				self.control.x += dx;
				self.control.y += dy;
			}, false);

			this.controlPoint3.addEventListener("mouseover", function(e){
				this.opacity = 0.6;
			});
			this.controlPoint3.addEventListener("mouseout", function(e){
				this.opacity = 0.1;
			});

		}
	

		var moveControlPoint = function(UIControlPoint, e, p){
			var dx = e.xrel / self._scale; //(e.x - UIControlPoint._x),
				dy = e.yrel / self._scale; //(e.y - UIControlPoint._y);

			UIControlPoint._x += dx;
			UIControlPoint._y += dy;
			UIControlPoint.x += dx;
			UIControlPoint.y += dy;

			//self["x"+p] += dx;
			//self["y"+p] += dy;

			self.w = Math.abs(self.x2 - self.x1);
			self.h = Math.abs(self.y2 - self.y1);

			self.g = {
				x : self.x1>=self.x2 ? self.x2 : self.x1,
				y : self.y1>=self.y2 ? self.y2 : self.y1
			};
		};

		this.controlPoint1.addEventListener("drag", function(e){
			moveControlPoint(this, e, 1);
		}, false);

		this.controlPoint2.addEventListener("drag", function(e){
			moveControlPoint(this, e, 2);
		}, false);

	},

	isPointInside : function(mx, my){
		var x1 = this.__x + this.x1*this._scale,
			y1 = this.__y + this.y1*this._scale,
			x2 = x1 + this.__w,
			y2 = y1 + this.__h;

		return	(mx>=x1 && mx<=x2 && my>=y1 && my<=y2) ? true : false;
	},

	draw : function(){
		this.w = Math.abs(this.x2 - this.x1);
		this.h = Math.abs(this.y2 - this.y1);

		
		this.g = {
			x : this.x1>=this.x2 ? this.x2 : this.x1,
			y : this.y1>=this.y2 ? this.y2 : this.y1
		};

		var params = {
				x : this._x,
				y : this._y,
				w : this.w,
				h : this.h,
			};

		this.x1 = this.controlPoint1.x;
		this.y1 = this.controlPoint1.y;
		this.x2 = this.controlPoint2.x;
		this.y2 = this.controlPoint2.y;
		
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
		canvas.lineWidth = 1;
		
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
	__construct : function(){
		this.radius = this.options.radius || 2;
		this.background = "rgba(255, 255, 255, 0.2)",
		this.lineWidth = 2;
		this.opacity = 0.5;
	},

	init : function(){
		var self = this;
		this.w = 12;
		this.h = 12;

		this.g = {
			x : - this.w/2,
			y : - this.h/2
		};

		this.addEventListener("mousedown", function(e){
			e.stopPropagation();
		}, true);

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

		canvas.roundbox(params.x-hx-1, params.y-hy-1, params.w+2, params.h+2, this.radius, '', 'rgba(0, 0, 0, 0.5)', this.lineWidth); // main view
		canvas.roundbox(params.x-hx, params.y-hy, params.w, params.h, this.radius, this.background, 'rgba(255, 255, 255, 0.5)', this.lineWidth); // main view

	}
});



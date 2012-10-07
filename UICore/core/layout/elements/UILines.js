/* -------------------------- */
/* Native (@) 2012 Stight.com */
/* -------------------------- */

Native.elements.export("UILine", {

	__construct : function(){
		this.updateParameters();
	},

	init : function(){
		var self = this,
			nbparams = 0,
			nbpoints = 0;

		this.vertices = OptionalValue(this.options.vertices, []);
		this.points = []; // array of points, relative coordinates to parent
		this.path = []; // array of points, absolute coordinates

		this.controlPoints = []; // array of Native Elements
		this.flags._canReceiveFocus = true;

		this.lineWidth = OptionalNumber(this.options.lineWidth, 1);

		this.displayControlPoints = OptionalBoolean(this.options.displayControlPoints, false);

		nbparams = this.vertices.length;

		if (nbparams%2 != 0 || nbparams < 4){
			throw "UILine: Incorrect number of points in vertices " + nbparams;
		}

		nbpoints = nbparams/2;

		for (var i=0; i<nbpoints; i++){
			var px = Number(this.vertices[2*i]),
				py = Number(this.vertices[2*i+1]);

			// relative coords
			this.points.push({
				x : px,
				y : py
			});

			// absolute coords
			this.path.push([
				this._x + px,
				this._y + py
			]);

			this.controlPoints[i] = this.add("UIControlPoint", {
				x : px,
				y : py,
				color : this.color,
				hidden : !this.displayControlPoints
			});

		}

		this.nbpoints = this.points.length;

		this.updateParameters = function(){
			for (var i=0; i<nbpoints; i++){
				this.points[i].x = this.controlPoints[i].x;
				this.points[i].y = this.controlPoints[i].y;

				this.path[i][0] = this._x + this.points[i].x;
				this.path[i][1] = this._y + this.points[i].y;
			}

			var first = 0,
				last = nbpoints-1;

			this.w = Math.abs(this.points[last].x - this.points[first].x);
			this.h = Math.abs(this.points[last].y - this.points[first].y);

			this.g = {
				x : this.points[first].x>=this.points[last].x ? this.points[last].x : this.points[first].x,
				y : this.points[first].y>=this.points[last].y ? this.points[last].y : this.points[first].y
			};
		};

		this.setControlPoint = function(i, point){
			this.controlPoints[i].x = point.x;
			this.controlPoints[i].y = point.y;
		};

		this.addEventListener("change", function(e){
			this.updateParameters();
		}, false);

		this.addEventListener("mousedown", function(e){
			this.focus();
			e.stopPropagation();
		}, false);
	},

	isPointInside : function(mx, my){
		return this.mouseOverPath;
	},

	draw : function(){
		this.updateParameters();

		var params = {
				x : this._x,
				y : this._y,
				w : this.w,
				h : this.h,
			},

			nbpoints = this.points.length;
		
		/*
		canvas.setColor("rgba(0, 0, 0, 0.5)");
		var r = {
			x : this._x + this.points[0].x,
			y : this._y + this.points[0].y,
			w : this.w,
			h : this.h
		}
		canvas.fillRect(r.x, r.y, r.w, r.h);
		*/

		canvas.strokeStyle = this.color;
		canvas.lineWidth = this.lineWidth;

		this.mouseOverPath = canvas.spline(this.path, canvas.mouseX, canvas.mouseY, this.lineWidth+8);

		/*		
		if (nbpoints==2){
			canvas.beginPath();
			canvas.moveTo(params.x + this.points[0].x, params.y + this.points[0].y);
			canvas.lineTo(params.x + this.points[1].x, params.y + this.points[1].y);
			canvas.stroke();
		}

		else if (nbpoints==3){
			canvas.beginPath();
			canvas.moveTo(params.x + this.points[0].x, params.y + this.points[0].y);
			canvas.quadraticCurveTo(
				params.x + this.points[1].x, params.y + this.points[1].y, 
				params.x + this.points[2].x, params.y + this.points[2].y
			);
			canvas.stroke();
		}

		else if (nbpoints==4){
			canvas.beginPath();
			canvas.moveTo(params.x + this.points[0].x, params.y + this.points[0].y);
			canvas.bezierCurveTo(
				params.x + this.points[1].x, params.y + this.points[1].y, 
				params.x + this.points[2].x, params.y + this.points[2].y,
				params.x + this.points[3].x, params.y + this.points[3].y
			);
			canvas.stroke();
		}

		else if (nbpoints>=5){
			this.mouseOverPath = canvas.spline(this.path, canvas.mouseX, canvas.mouseY);
		}
		*/

	}
});

Native.elements.export("UIControlPoint", {
	init : function(){
		var self = this;
		this.w = 16;
		this.h = 16;

		this.radius = OptionalNumber(this.options.radius, 3);
		this.background = OptionalValue(this.options.background, "rgba(0, 0, 0, 0.5)"),
		this.hidden = OptionalBoolean(this.options.hidden, false),
		this.lineWidth = 1;
		this.opacity = 0.5;

		this.g = {
			x : - this.w/2,
			y : - this.h/2
		};

		this.addEventListener("mousedown", function(e){
			this.opacity = 1.00;
			this.parent.focus();
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

		if (this.parent.hasFocus && !this.hidden){
			canvas.roundbox(params.x-hx+2, params.y-hy+2, params.w-4, params.h-4, this.radius, '', "#000000", this.lineWidth); // main view
			canvas.roundbox(params.x-hx+4, params.y-hy+4, params.w-8, params.h-8, 0, this.background, "#ffffff", this.lineWidth); // main view
		}

	}
});



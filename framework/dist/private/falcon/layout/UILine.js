/* ------------------------+------------- */
/* Native Framework 2.0    | Falcon Build */
/* ------------------------+------------- */
/* (c) 2013 Stight.com - Vincent Fontaine */
/* -------------------------------------- */

Native.elements.export("UILine", {
	init : function(){
		var self = this,
			o = this.options,
			nbparams = 0,
			nbpoints = 0;

		this.canReceiveFocus = true;
		this.vertices = OptionalValue(o.vertices, []);
		this.displayControlPoints = OptionalBoolean(o.displayControlPoints, false);
		this.lineWidth = OptionalNumber(o.lineWidth, 1);
		this.controlPoints = []; // array of UIControlPoint

		this.setVertex = function(i, vertex){
			this.controlPoints[i].left = vertex.x - 8;
			this.controlPoints[i].top = vertex.y - 8;
		};

		this.getVertices = function(){
			var vertices = [];
			for (var i=0; i<this.points.length; i++){
				vertices.push(this.points[i][0]);
				vertices.push(this.points[i][1]);
			}
			return vertices;
		};

		this.setVertices = function(vertices){
			var nbparams = vertices.length;

			if (nbparams%2 != 0 || nbparams < 4){
				throw "UILine: Incorrect number of points in vertices " + nbparams;
			}

			this.points = []; // array of points

			for (var i=0; i<nbparams/2; i++){
				var px = Number(vertices[2*i+0]),
					py = Number(vertices[2*i+1]);
				// coordinates of vertex i
				this.points.push([px, py]);

				if (this.controlPoints[i]) {
					this.setVertex(i, {
						x : px,
						y : py
					});
				} else {
					this.controlPoints[i] = this.add("UIControlPoint", {
						left : px - 8,
						top : py - 8,
						color : this.color,
						hidden : !this.displayControlPoints
					});
				}
			}

			this.nbpoints = this.points.length;
			this.pointStep = this.getNormalizedStepBetweenPoints();
		};

		this.refreshElement = function(){
			for (var i=0; i<this.points.length; i++){
				this.points[i][0] = this.controlPoints[i].x;
				this.points[i][1] = this.controlPoints[i].y;
			}

			this._needRefresh = true;
			this._needRedraw = true;
		};

		this.addEventListener("controlpointupdate", function(e){
			//this.refreshElement();
		}, false);

		this.addEventListener("mousedown", function(e){
			this.focus();
			e.stopPropagation();
		}, false);

		this.getNormalizedStepBetweenPoints = function(){
			var n = this.points.length-1,
				d = 0;

			for (var i=0; i<n; i++) {
				d += Math.distance(
					this.points[i][0], this.points[i][1],
					this.points[i+1][0], this.points[i+1][1]
				);
			}

			return 1/d;
		};

		this.getSplinePoint = function(t){
			var n = this.points.length-1,
				B = Math.spline,
				p = [0, 0];

			for (var i=0; i<=n; i++){
				p[0] += this.points[i][0] * B(i, n, t);
				p[1] += this.points[i][1] * B(i, n, t);
			}
			return p;
		};

		this.getPathBounds = function(){
			var bx = [],
				by = [],
				p = [0, 0],
				k = this.lineWidth/2,
				step = this.getNormalizedStepBetweenPoints();

			for (var t=0; t<=1; t+=step){
				p = this.getSplinePoint(t);
				bx.push(p[0]);
				by.push(p[1]);
			}

			return {
				left : Math.floor(1-k + bx.min()),
				top : Math.floor(1-k + by.min()),
				right : Math.floor(k + bx.max()),
				bottom : Math.floor(k + by.max())
			}
		};

		this.setVertices(this.vertices);

	},

	isPointInside : function(mx, my){
		return this.mouseOverPath;
	},

	draw : function(context){
		var	params = this.getDrawingBounds(),
			nbpoints = this.points.length;

		DOMElement.draw.box(this, context, params);
		
		context.strokeStyle = this.color;
		context.lineWidth = this.lineWidth;

		var b = context.spline(
			this.points,
			window.mouseX, 
			window.mouseY, 
			this.lineWidth+8
		);

		//var b = this.getPathBounds();
		//var b = context.getPathBounds();

		this.boundingRect = {
			left : b.left,
			top : b.top,
			width : (b.right - b.left),
			height : (b.bottom - b.top)
		};

	/*		
		if (nbpoints==2){
			context.beginPath();
			
			context.moveTo(
				params.x + this.points[0][0], 
				params.y + this.points[0][1]
			);
			
			context.lineTo(
				params.x + this.points[1][0], 
				params.y + this.points[1][1]
			);
			
			context.stroke();
		}

		else if (nbpoints==3){
			context.beginPath();
			
			context.moveTo(
				params.x + this.points[0][0], 
				params.y + this.points[0][1]
			);

			context.quadraticCurveTo(
				params.x + this.points[1][0], params.y + this.points[1][1], 
				params.x + this.points[2][0], params.y + this.points[2][1]
			);
			
			context.stroke();
		}

		else if (nbpoints==4){
			context.beginPath();
			
			context.moveTo(
				params.x + this.points[0][0], 
				params.y + this.points[0][1]
			);
			
			context.bezierCurveTo(
				params.x + this.points[1][0], params.y + this.points[1][1], 
				params.x + this.points[2][0], params.y + this.points[2][1],
				params.x + this.points[3][0], params.y + this.points[3][1]
			);
			
			context.stroke();
		}

		else if (nbpoints>=5){
			this.mouseOverPath = context.spline(
				this.points, 
				window.mouseX,
				window.mouseY
			);
		}
	*/
	}
});

Native.elements.export("UIControlPoint", {
	public : {
		left : {
			set : function(value){
				this.refreshElement();
				if (this.parent.refreshElement) this.parent.refreshElement();
				this.parent.fireEvent("controlpointupdate");
			}
		},
		
		top : {
			set : function(value){
				this.refreshElement();
				if (this.parent.refreshElement) this.parent.refreshElement();
				this.parent.fireEvent("controlpointupdate");
			}
		}
	},

	init : function(){
		var self = this,
			o = this.options;

		this.width = 16;
		this.height = 16;

		this.radius = OptionalNumber(o.radius, 8);
		this.background = OptionalValue(o.background, "rgba(0, 0, 0, 0.1)");

		this.hidden = OptionalBoolean(o.hidden, false);
		this.lineWidth = 1.5;
		this.opacity = 0.5;

		this.refreshElement = function(){
			this.x = this._left + this._width/2;
			this.y = this._top + this._height/2;
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
			this.left += e.dx;
			this.top += e.dy;
			this.opacity = 1.00;

			if (this.parent.boundingRect){
				this.parent.shrink(this.parent.boundingRect);
				this.parent.alpha += 0.0001;
			}

			e.stopPropagation();
		}, false);

		this.addEventListener("dragend", function(e){
			this.set("opacity", 0.50, 90);
		});

		this.refreshElement();
	},

	draw : function(context){
		var	params = this.getDrawingBounds();

		if (!this.hidden){
			context.roundbox(
				params.x+4, params.y+4, 
				params.w-8, params.h-8, 
				this.radius, 'rgba(255, 255, 255, 0.6)', "rgba(128, 128, 128, 0.6)", this.lineWidth
			);

			context.roundbox(
				params.x, params.y, 
				params.w, params.h,
				this.radius, this.background, "rgba(128, 128, 128, 0.9)", this.lineWidth
			);
		}
	}
});



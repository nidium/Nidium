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

		this.points = []; // array of points
		this.controlPoints = []; // array of UIControlPoint

		nbparams = this.vertices.length;

		if (nbparams%2 != 0 || nbparams < 4){
			throw "UILine: Incorrect number of points in vertices " + nbparams;
		}

		nbpoints = nbparams/2;

		for (var i=0; i<nbpoints; i++){
			var px = Number(this.vertices[2*i+0]),
				py = Number(this.vertices[2*i+1]);

			// coordinates of vertex i
			this.points.push([px, py]);

			this.controlPoints[i] = this.add("UIControlPoint", {
				left : px - 8,
				top : py - 8,
				color : this.color,
				hidden : !this.displayControlPoints
			});
		}

		this.nbpoints = this.points.length;

		this.refreshElement = function(){
			for (var i=0; i<this.points.length; i++){
				this.points[i][0] = this.controlPoints[i].x;
				this.points[i][1] = this.controlPoints[i].y;
			}

			this._needRefresh = true;
			this._needRedraw = true;
		};

		this.setVertex = function(i, vertex){
			this.controlPoints[i].left = vertex.x - 8;
			this.controlPoints[i].top = vertex.y - 8;
		};

		this.addEventListener("update", function(e){
			//this.refreshElement();
		}, false);

		this.addEventListener("mousedown", function(e){
			this.focus();
			e.stopPropagation();
		}, false);

		this.getBoundingRect = function(){
		};

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
			}
		},
		
		top : {
			set : function(value){
				this.refreshElement();
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
			if (this.parent.refreshElement) this.parent.refreshElement();
			this.parent.fireEvent("update");
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



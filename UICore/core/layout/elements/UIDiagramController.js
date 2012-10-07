/* -------------------------- */
/* Native (@) 2012 Stight.com */
/* -------------------------- */

Native.elements.export("UIDiagramController", {
	init : function(){
		var self = this;

		this.background = OptionalValue(this.options.background, 'rgba(0, 0, 10, 0.4)');
		this.color = OptionalValue(this.options.color, "#ffffff");
		this.name = OptionalString(this.options.name, "Default");


		this.setLinkPosition = function(link){
			var	startPoint = {
					x : link.controlPoints[0].x,
					y : link.controlPoints[0].y
				},

				endPoint = {
					x : link.controlPoints[4].x,
					y : link.controlPoints[4].y
				};

			this.updateLink(link, startPoint, endPoint);
		};

		this.updateLink = function(link, startPoint, endPoint){
			var	dx = diagramController.__x,
				dy = diagramController.__y,

				startPoint = {
					x : this.__x - dx,
					y : this.__y + this.__h/2 - dy
				},

				endPoint = {
					x : end.x - dx,
					y : end.y - dy
				},

				sx1 = 0, sy1 = 0,
				sx2 = 0, sy2 = 0,
				sx3 = 0, sy3 = 0;


			if (this.pintype == "output") {
				startPoint.x += this.__w;
			}

			if (this.pintype == "output") {
				sx1 = startPoint.x + Math.abs(endPoint.x - startPoint.x)/2;
				sy1 = startPoint.y;
			} else {
				sx1 = startPoint.x - Math.abs(startPoint.x - endPoint.x)/2;
				sy1 = startPoint.y;
			}

			sx2 = sx1;
			sy2 = sy1 - (sy1 - endPoint.y)/2;

			sx3 = sx2;					
			sy3 = endPoint.y;

			/* ----------------------------------------------- */
			
			link.setControlPoint(0, startPoint);

			link.setControlPoint(1, {
				x : sx1, 
				y : sy1
			});

			link.setControlPoint(2, {
				x : sx2, 
				y : sy2
			});

			link.setControlPoint(3, {
				x : sx3, 
				y : sy3
			});

			link.setControlPoint(4, endPoint);
		};


	},

	draw : function(){
		var params = {
				x : this._x,
				y : this._y,
				w : this.w,
				h : this.h
			};

		if (this.shadowBlur != 0) {
			canvas.setShadow(0, 0, this.shadowBlur, "rgba(0, 0, 0, 0.5)");
			canvas.roundbox(params.x, params.y, params.w, params.h, this.radius, this.background, false);
			canvas.setShadow(0, 0, 0);
		} else {
			canvas.roundbox(params.x, params.y, params.w, params.h, this.radius, this.background, false);
		}

	}
});
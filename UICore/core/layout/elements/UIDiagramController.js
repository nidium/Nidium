/* -------------------------- */
/* Native (@) 2012 Stight.com */
/* -------------------------- */

Native.elements.export("UIDiagramController", {
	init : function(){
		var self = this;

		this.background = OptionalValue(this.options.background, 'rgba(0, 0, 10, 0.4)');
		this.color = OptionalValue(this.options.color, "#ffffff");
		this.name = OptionalString(this.options.name, "Default");

		this.getPinConnectionPoint = function(pin){
			var	dx = this.__x,
				dy = this.__y;

			return {
				x : pin.__x + (pin.pintype == "output" ? pin.__w : 0),
				y : pin.__y + pin.__h/2
			}
		};

		this.setLinkPosition = function(pin, link){
				endPoint = {
					x : link.controlPoints[4].x,
					y : link.controlPoints[4].y
				};
			var points = this.getUILineVertives(pin.pintype, endPoint);
			this.setUILineVertices(points);
		};

		this.getUILineVertives = function(pintype, absStartPoint, absEndPoint){
			var	dx = this.__x,
				dy = this.__y,

				startPoint = {
					x : absStartPoint - dx,
					y : absStartPoint - dy
				},

				endPoint = {
					x : absEndPoint.x - dx,
					y : absEndPoint.y - dy
				},

				sx1 = 0, sy1 = 0,
				sx2 = 0, sy2 = 0,
				sx3 = 0, sy3 = 0;

			if (pintype == "output") {
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

			return {
				sx0 : startPoint.x,
				sy0 : startPoint.y,

				sx1 : sx1,
				sy1 : sy1,

				sx2 : sx2,
				sy2 : sy2,

				sx3 : sx3,
				sy3 : sy3,

				sx4 : endPoint.x,
				sy4 : endPoint.y
			};
		};

		this.setUILineVertices = function(UILine, p){
			UILine.setPoint(0, {
				x : p.sx1,
				y : p.sy1
			});
	
			UILine.setPoint(1, {
				x : p.sx1,
				y : p.sy1
			});

			UILine.setPoint(2, {
				x : p.sx2,
				y : p.sy2
			});

			UILine.setPoint(3, {
				x : p.sx3,
				y : p.sy3
			});

			UILine.setPoint(4, {
				x : p.sx4,	
				y : p.sy4
			});
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
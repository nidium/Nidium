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
			//var points = this.getUILineVertives(pin.pintype, endPoint);
			//this.setUILineVertices(points);
		};

		this.links = [];

		this.connect = function(sourcePin, targetPin){
			var absStartPoint = this.getPinConnectionPoint(sourcePin),
				absEndPoint = this.getPinConnectionPoint(targetPin);

			var v = this.getUILineVertives(sourcePin.pintype, absStartPoint, absEndPoint);

			var link = this.add("UILine", {
					vertices : [
						v.sx0, v.sy0,
						v.sx1, v.sy1,
						v.sx2, v.sy2,
						v.sx3, v.sy3,
						v.sx4, v.sy4
					],
					displayControlPoints : true,
					color : "#ff0000",
					lineWidth : 3
				});

			this.links.push({
				source : sourcePin,
				target : targetPin,
				link : link
			});

		};


		this.getUILineVertives = function(pintype, absStartPoint, absEndPoint){
			var	dx = this.__x,
				dy = this.__y,

				startPoint = {
					x : absStartPoint.x - dx,
					y : absStartPoint.y - dy
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

		this.setUILineVertices = function(UILine, v){
			UILine.setVertex(0, {
				x : v.sx0,
				y : v.sy0
			});

			UILine.setVertex(1, {
				x : v.sx1,
				y : v.sy1
			});

			UILine.setVertex(2, {
				x : v.sx2,
				y : v.sy2
			});

			UILine.setVertex(3, {
				x : v.sx3,
				y : v.sy3
			});

			UILine.setVertex(4, {
				x : v.sx4,	
				y : v.sy4
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
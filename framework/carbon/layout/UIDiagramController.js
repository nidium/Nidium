/* -------------------------- */
/* Native (@) 2012 Stight.com */
/* -------------------------- */

Native.elements.export("UIDiagramController", {
	init : function(){
		var self = this;

		this.background = OptionalValue(this.options.background, 'rgba(0, 0, 10, 0.4)');
		this.color = OptionalValue(this.options.color, "#ffffff");
		this.name = OptionalString(this.options.name, "Default");

		this.links = new Set(); // link repository set

		this.getPinConnectionPoint = function(pin){
			var	dx = this.__x,
				dy = this.__y;

			return {
				x : pin.__x + (pin.pintype == "output" ? pin.__w : 0),
				y : pin.__y + pin.__h/2
			}
		};

		this.getLinkEvent = function(sourcePin, targetPin){
			var source = sourcePin.getParentDiagram(),
				target = targetPin.getParentDiagram();

			source.pin = sourcePin;
			target.pin = targetPin;
			
			return {
				source : source,
				target : target
			};
		};

		this.getLink = function(sourcePin, targetPin){
			var linkset = this.links,
				r = null;

			for (var link of linkset){
				var s = link.sourcePin,
					t = link.targetPin;

				if (sourcePin == s && targetPin == t){
					r = link;
					break;
				}
			}
			return r;
		};

		this.reset = function(){
			var linkset = this.links;

			for (var link of linkset){
				self.removeLink(link);
			}
		};

		this.removeLink = function(link){
			var event = this.getLinkEvent(link.sourcePin, link.targetPin);
			this.fireEvent("disconnect", event, function(){

				var sourcePin = link.sourcePin,
					targetPin = link.targetPin;

				sourcePin.connections.delete(targetPin);
				targetPin.connections.delete(sourcePin);

				this.links.delete(link);
				link.fadeOut(50, function(){
					this.remove();
				});
			});
		};

		this.disconnect = function(sourcePin, targetPin){
			var link = this.getLink(sourcePin, targetPin);
			this.removeLink(link);
		};

		this.connect = function(sourcePin, targetPin, opt){
			var options = opt || {},
				event = this.getLinkEvent(sourcePin, targetPin),
				color = OptionalValue(options.color, "#ff0000"),
				lineWidth = OptionalNumber(options.lineWidth, 3);

			if (sourcePin.connectedTo(targetPin)) {
				this.fireEvent("alreadyconnected", event);
				return false;
			}

			this.fireEvent("connect", event, function(){
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
					color : color,
					lineWidth : lineWidth
				});

				link.controlPoints[4]._diagram = sourcePin.getParentDiagram();
				link.controlPoints[4]._pin = sourcePin;

				link.sourcePin = sourcePin;
				link.targetPin = targetPin;
				
				self.links.add(link);

				link.addEventListener("change", function(e){
					var absStartPoint = self.getPinConnectionPoint(sourcePin),
						absEndPoint = {
							x : e.x,
							y : e.y
						};

					self.updateUILine(this, sourcePin.pintype, absStartPoint, absEndPoint);
				}, false);

				link.addEventListener("keydown", function(e){
					if (!this.hasFocus) return false;

					switch (e.keyCode) {
						case 8 :
						case 127 : // backspace, delete
							self.removeLink(link);
							break;
					}

				}, false);

				sourcePin.connections.add(targetPin);
				targetPin.connections.add(sourcePin);
			});
		};

		this.updatePinLinks = function(pin){
			var linkset = this.links;

			for (var link of linkset){
				var sourcePin = link.sourcePin,
					targetPin = link.targetPin;

				if (sourcePin == pin || targetPin == pin) {
					this.updateLinkPosition(link, sourcePin, targetPin);
				}
			}
		};

		this.deletePinLinks = function(pin){
			var linkset = this.links;

			for (var link of linkset){
				var sourcePin = link.sourcePin,
					targetPin = link.targetPin;

				if (sourcePin == pin || targetPin == pin) {
					this.removeLink(link);
				}
			}
		};

		this.updateLinkPosition = function(link, sourcePin, targetPin){
			var absStartPoint = this.getPinConnectionPoint(sourcePin),
				absEndPoint = this.getPinConnectionPoint(targetPin);

			this.updateUILine(link, sourcePin.pintype, absStartPoint, absEndPoint);
		};

		this.updateUILine = function(UILine, pintype, absStartPoint, absEndPoint){
			var	vertices = this.getUILineVertives(pintype, absStartPoint, absEndPoint);
			this.setUILineVertices(UILine, vertices);
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

			if (endPoint.x<=startPoint.x && pintype=="output") {
				sx1 = sx1 + 0.25*(startPoint.x-endPoint.x);
				sx2 = sx1 - 1.50*(startPoint.x-endPoint.x);
				sx3 = sx2 - (startPoint.x-endPoint.x);

				if (endPoint.y>startPoint.y) {
					sy1 = sy1 + Math.abs(startPoint.y-endPoint.x)/8;
					sy3 = sy3 - Math.abs(startPoint.y-endPoint.x)/8;
				}
			}

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
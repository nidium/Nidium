/* -------------------------- */
/* Native (@) 2012 Stight.com */
/* -------------------------- */

Native.elements.export("UIDiagram", {
	init : function(){
		var self = this,
			diagramController = self.parent ? self.parent : self,
			y = 0,
			pins = OptionalValue(this.options.pins, []),
			nbpins = pins.length;

		this.flags._canReceiveFocus = true;

		/* --------------------- */

		this.color = OptionalValue(this.options.color, "#ffffff");
		this.name = OptionalString(this.options.name, "Default");

		this.shadowBlur = OptionalNumber(this.options.shadowBlur, 12);
		this.shadowColor = OptionalValue(this.options.shadowColor, "rgba(0, 0, 0, 0.5)");
		this.backgroundBlur = 0; //OptionalNumber(this.options.backgroundBlur, 0);

		this.movable = OptionalBoolean(this.options.movable, true);
		this.resizable = OptionalBoolean(this.options.resizable, false);
		this.closeable = OptionalBoolean(this.options.closable, true);

		this.radius = OptionalNumber(this.options.radius, 5);
		this.fontSize = OptionalNumber(this.options.fontSize, 9);

		this.pinLabelHeight = OptionalNumber(this.options.pinLabelHeight, 16);
		this.pinFontSize = OptionalNumber(this.options.pinFontSize, 9);

		/* --------------------- */

		canvas.setFontSize(this.fontSize);
		this.w = 10 + Math.round(canvas.measureText(this.label)) + 10 + (this.closeable?16:0) + 26;
		this.h = OptionalNumber(this.options.h, 24 + nbpins*this.pinLabelHeight + 4);

		this.pins = [];

		/* --------------------- */

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
				canvas.fillText(p.label, p._x+textOffsetX, p._y+textOffsetY);
			}
		});

		if (this.movable) {
			this.handle.addEventListener("dragstart", function(e){
				self.set("scale", 1.1, 80);
				//self.set("backgroundBlur", 1, 80);
				self.set("shadowBlur", 20, 70);
				self.shadowColor = "rgba(0, 0, 0, 0.95)";
				e.stopPropagation();
			}, false);

			this.handle.addEventListener("drag", function(e){
				this.parent.left += e.xrel;
				this.parent.top += e.yrel;
				self.updatePins();
				e.stopPropagation();
			});

			this.handle.addEventListener("dragend", function(e){
				self.set("scale", 1, 50);
				//self.set("backgroundBlur", 0, 50);
				self.set("shadowBlur", OptionalNumber(this.options.shadowBlur, 12), 50);
				self.shadowColor = self.options.shadowColor || "rgba(0, 0, 0, 0.5)";
				e.stopPropagation();
			}, false);
		}

		if (this.closeable) {
			this.handle.closeButton = this.add("UIButtonClose", {
				x : this.w-19,
				y : 4,
				w : 16,
				h : 16,
				background : "rgba(0, 0, 0, 0.75)",
				color : "#888888"
			});

			this.handle.closeButton.addEventListener("mouseup", function(e){
				self.fireEvent("close", e, function(){
					self.set("scale", 0, 120, function(){});
					self.shadowBlur = 6;
					self.shadowColor = "rgba(0, 0, 0, 0.20)";
					self.closeDiagram();
					e.stopPropagation();
				});
			}, false);
		}

		this.contentView = this.add("UIView", {
			x : 3,
			y : 24,
			w : self.w-6,
			h : nbpins*this.pinLabelHeight + 1,
			radius : 4,
			background : "#ffffff",
			color : "#333333"
		});

		/* --------------------- */

		this.getPin = function(pinnum){
			return this.pins[pinnum] ? this.pins[pinnum] : undefined;
		};

		this.updatePins = function(){
			for (var i=0; i<nbpins; i++){
				var	pin = this.pins[i];
				diagramController.updatePinLinks(pin);
			}
		};

		this.closeDiagram = function(){
			
			for (var i=0; i<nbpins; i++){
				var	pin = this.pins[i];
				diagramController.deletePinLinks(pin);
			}
		};

		this.attachAllTheGoodThingsToPin = function(pin){

			pin.connections = new Set();
			pin.targetLinks = [];

			pin.getParentDiagram = function(){
				return self;
			};

			pin.connectedTo = function(targetPin){
				return pin.connections.has(targetPin) || targetPin.connections.has(pin);
			};

			pin.addEventListener("dragstart", function(e){
				var absStartPoint = diagramController.getPinConnectionPoint(pin),
					absEndPoint = {
						x : e.x,
						y : e.y
					};

				var v = diagramController.getUILineVertives(pin.pintype, absStartPoint, absEndPoint);

				pin.link = diagramController.add("UILine", {
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

				pin.link.bringToTop();
				pin.link.controlPoints[4]._diagram = self;
				pin.link.controlPoints[4]._pin = pin;

				e.stopPropagation();
			}, false)



			pin.addEventListener("drag", function(e){
				var absStartPoint = diagramController.getPinConnectionPoint(pin),
					absEndPoint = {
						x : e.x,
						y : e.y
					};

				pin.link.focus();
				diagramController.updateUILine(pin.link, pin.pintype, absStartPoint, absEndPoint);

				e.stopPropagation();
			}, false)


			pin.addEventListener("dragend", function(e){
				pin.link.remove();
				pin.link = null;
			}, false)



			pin.getLink = function(e){
				var src = e.source,
					dst = this,
					stype = src._pin ? src._pin.pintype : null,
					dtype = dst._pin.pintype;

				if (!src._diagram || !src._pin) return false;
				if (src._diagram == dst._diagram) return false;
				if ((stype == "input" || stype == "controller") && dtype == "input") return false;
				if (stype == "output" && dtype == "output") return false;

				return diagramController.getLinkEvent(src._pin, dst._pin);
			};


			pin.addEventListener("dragenter", function(e){
				var link = this.getLink(e);
				if (!link) return false;

				self.parent.fireEvent("pinEnter", link);
				e.stopPropagation();
			}, false);

			pin.addEventListener("dragover", function(e){
				var link = this.getLink(e);
				if (!link) return false;

				self.parent.fireEvent("pinOver", link);
				e.stopPropagation();
			}, false);

			pin.addEventListener("dragleave", function(e){
				var link = this.getLink(e);
				if (!link) return false;
				
				self.parent.fireEvent("pinLeave", link);
				e.stopPropagation();
			}, false);

			pin.addEventListener("drop", function(e){
				var link = this.getLink(e);
				if (!link) return false;

				self.parent.fireEvent("pinDrop", link, function(){
					self.parent.fireEvent("connect", link, function(){
						diagramController.connect(link.source.pin, link.target.pin);
					});
				});

				e.stopPropagation();
			}, false);

		};

		this._addPin = function(i, options, y){
			var label = OptionalString(options.label, "pin"+i),
				type = OptionalString(options.type, "input"),
				color = "#333333",
				background = "",
				textAlign = "left",
				txt = "";

			switch(type){
				case "input" : 
					color = "#333333";
					textAlign = "left";
					txt = '- ' + label;
					break;

				case "output" : 
					color = "#333333";
					textAlign = "right";
					txt = label + ' -';
					break;

				case "controller" :
					color = "#550000";
					textAlign = "left";
					txt = '- ' + label;
					break;
			}

			this.pins[i] = this.contentView.add("UILabel", {
				x : 0,
				y : y,
				w : self.w-6,
				h : self.pinLabelHeight,
				name : "pin" + i,
				label : txt,
				lineHeight : self.pinLabelHeight,
				fontSize : self.pinFontSize,
				textAlign : textAlign,
				background : '',
				color : color
			});

			this.pins[i].pinnum = i;
			this.pins[i].pintype = type;
			this.pins[i]._diagram = self;
			this.pins[i]._pin = this.pins[i];

			this.attachAllTheGoodThingsToPin(this.pins[i]);

		};


		for (var i=0; i<nbpins; i++){
			this._addPin(i, pins[i], y);
			y += this.pinLabelHeight;
		}

		/* --------------------- */

		if (this.resizable) {
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

		if (this.backgroundBlur){
			this.blurbox = {
				x : this.__x,
				y : this.__y,
				w : this.__w,
				h : this.handle.__h + 1
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
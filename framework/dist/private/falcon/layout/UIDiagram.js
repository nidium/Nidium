/* ------------------------+------------- */
/* Native Framework 2.0    | Falcon Build */
/* ------------------------+------------- */
/* (c) 2013 nidium.com - Vincent Fontaine */
/* -------------------------------------- */

Native.elements.export("UIDiagram", {
	init : function(){
		var self = this,
			o = this.options,
			context = this.layer.context,
			diagramController = self.parent ? self.parent : self,
			y = 0,
			pins = OptionalValue(o.pins, []),
			nbpins = pins.length;

		/* --------------------- */

		this.setProperties({
			canReceiveFocus : true,
			color : OptionalValue(o.color, "#ffffff"),
			shadowBlur : OptionalNumber(o.shadowBlur, 8),
			shadowColor : OptionalValue(o.shadowColor, "rgba(0, 0, 0, 0.5)"),
			radius : OptionalNumber(o.radius, 5),
			fontSize : OptionalNumber(o.fontSize, 9),

			pinLabelHeight : OptionalNumber(o.pinLabelHeight, 16),
			pinFontSize : OptionalNumber(o.pinFontSize, 9),

			movable : OptionalBoolean(o.movable, true),
			resizable : OptionalBoolean(o.resizable, false),
			closeable : OptionalBoolean(o.closable, true),

			background : OptionalValue(o.background, 'rgba(255, 0, 10, 0.4)')
		});

		this.width = 150;
		this.height = OptionalNumber(o.height, 24 + nbpins*this.pinLabelHeight + 4);

		/* --------------------- */

		this.pins = [];

		/* --------------------- */

		this.unselect = function(){
			/*
			self.set("scale", 1, 50);
			self.set(
				"shadowBlur", 
				OptionalNumber(o.shadowBlur, 8), 
				12
			);
			self.shadowColor = OptionalValue(
				o.shadowColor,
				"rgba(0, 0, 0, 0.5)"
			);
			*/
		};

		this.addEventListener("mousedown", function(e){
			this.bringToFront();
			e.stopPropagation();
		}, false);

		this.addEventListener("mouseover", function(e){
			e.stopPropagation();
		}, false);

		this.handle = this.add("UIElement", {
			left : 0,
			top : 0,
			width : self.width,
			height : 24,
			radius : 4,
			background : "rgba(0, 0, 0, 0.05)",
			color : "#888888"
		});

		this.labelElement = this.add("UILabel", {
			left : 4,
			top : 0,
			height : 24,
			color : self.color,
			label : self.label,
			fontSize : 9
		});

		this.labelElement.addEventListener("dragstart", function(e){
			e.forcePropagation();
		}, true);

		if (this.movable) {
			this.handle.addEventListener("mousedown", function(e){
				self._mdownhandleStarted = true;
			}, false);

			this.handle.addEventListener("drag", function(e){
				self.left += e.xrel;
				self.top += e.yrel;
				self.updatePins();
				e.stopPropagation();
			});

			this.handle.addEventListener("dragend", function(e){
				self._mdownhandleStarted = false;
				self.unselect();
			}, false);

			this.handle.addEventListener("mouseup", function(){
				if (!self._mdownhandleStarted) return false;
				self._mdownhandleStarted = false;
				self.unselect();
			}, false);
		}

		if (this.closeable) {
			this.handle.closeButton = this.add("UIButtonClose", {
				left : this.width-19,
				top : 4,
				width : 16,
				height : 16,
				background : "rgba(0, 0, 0, 0.75)",
				color : "#888888"
			});

			this.handle.closeButton.addEventListener("mouseup", function(e){
				self.fireEvent("close", e, function(){
					self.fadeOut(120, function(){});
					self.closeDiagram();
					e.stopPropagation();
				});
			}, false);
		}

		this.contentView = this.add("UIView", {
			left : 3,
			top : 24,
			width : self.width-6,
			height : nbpins*this.pinLabelHeight + 1,
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
				return pin.connections.has(targetPin) || 
					   targetPin.connections.has(pin);
			};

			pin.addEventListener("dragstart", function(e){
				var absStartPoint = diagramController.getPinConnectionPoint(pin),
					absEndPoint = {
						x : e.x,
						y : e.y
					};

				var v = diagramController.getUILineVertives(
					pin.pintype, absStartPoint, absEndPoint
				);

				pin.link = diagramController.add("UILine", {
					vertices : [
						v.sx0, v.sy0,
						v.sx1, v.sy1,
						v.sx2, v.sy2,
						v.sx3, v.sy3,
						v.sx4, v.sy4
					],
					displayControlPoints : true,
					//background : "rgba(88, 0, 0, 0.3)",
					color : "#ff0000",
					lineWidth : 3
				});

				//pin.link.bringToFront();
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
				diagramController.updateUILine(
					pin.link, pin.pintype,
					absStartPoint, absEndPoint
				);

				e.stopPropagation();
			}, false)


			pin.addEventListener("dragend", function(e){
				pin.link.remove();
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
						diagramController.connect(
							link.source.pin,
							link.target.pin
						);
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
				left : 0,
				top : y,
				width : self.width-6,
				height : self.pinLabelHeight,
				name : "pin" + i,
				label : txt,
				lineHeight : self.pinLabelHeight,
				fontSize : self.pinFontSize,
				textAlign : textAlign,
				background : '',
				textShadowBlur : 0,
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

	draw : function(context){
		var	params = this.getDrawingBounds();

		NDMElement.draw.enableShadow(this);
		NDMElement.draw.box(this, context, params);
		NDMElement.draw.disableShadow(this);

		var gradient = context.createLinearGradient(
			params.x, params.y, 
			params.x, params.y+24
		);
		gradient.addColorStop(0.00, 'rgba(255, 255, 255, 0.20)');
		gradient.addColorStop(0.10, 'rgba(255, 255, 255, 0.05)');
		gradient.addColorStop(0.90, 'rgba(255, 255, 255, 0.00)');
		NDMElement.draw.box(this, context, params, gradient);
	}
});
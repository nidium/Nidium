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
				self.set("scale", 0, 120, function(){});
				self.shadowBlur = 6;
				self.shadowColor = "rgba(0, 0, 0, 0.20)";
				e.stopPropagation();
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

		this.updatePins = function(){
			for (var i=0; i<nbpins; i++){
				var	pin = this.pins[i];

				/*
				for (var l in pin.links){
					diagramController.setLinkPosition(pin.links[l]);
				}
				*/
			}
		};

		this.attachAllTheGoodThingsToPin = function(pin){

			pin.setPoint = function(i, p){
				pin.link.setControlPoint(i, p);
			};

			pin.getControlPoints = function(end){
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

			pin.updateLink = function(endPoint){
				var	p = this.getControlPoints(endPoint);
			
				this.setPoint(1, {
					x : p.sx1,
					y : p.sy1
				});

				this.setPoint(2, {
					x : p.sx2,
					y : p.sy2
				});

				this.setPoint(3, {
					x : p.sx3,
					y : p.sy3
				});

				this.setPoint(4, {
					x : p.sx4,	
					y : p.sy4
				});
			};


			pin.addEventListener("dragstart", function(e){
				var p = this.getControlPoints(e);

				pin.link = diagramController.add("UILine", {
					vertices : [
						p.sx0, p.sy0,
						p.sx1, p.sy1,
						p.sx2, p.sy2,
						p.sx3, p.sy3,
						p.sx4, p.sy4
					],
					displayControlPoints : true,
					color : "#ff0000",
					lineWidth : 3
				});

				pin.link.bringToTop();
				pin.link.controlPoints[4]._diagram = self;
				pin.link.controlPoints[4]._pin = pin;

				pin.link.addEventListener("change", function(e){
					pin.updateLink(e);
				}, false)


				e.stopPropagation();
			}, false)


			pin.addEventListener("drag", function(e){
				pin.link.focus();
				pin.updateLink(e);
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

				src._diagram.pin = src._pin;
				dst._diagram.pin = dst._pin;
				
				return {
					source : src._diagram,
					target : dst._diagram
				};
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

				self.parent.fireEvent("pinDrop", link);
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
/* -------------------------- */
/* Native (@) 2012 Stight.com */
/* -------------------------- */

Native.elements.export("UIWindow", {
	public : {
		width : {
			set : function(value){
				if (this.width<22) this.width = 22;
				this.handle.width = this.width;
				this.handle.closeButton.left = this.handle.width - 19;
				this.contentView.width = this.width - 6;
				this.resizer.updateElement();
			}
		},

		height : {
			set : function(value){
				if (this.height<40) this.height = 40;
				this.contentView.height = this.height - 27;
				this.resizer.updateElement();
			}
		}
	},


	init : function(){
		var self = this,
			o = this.options;

		this.canReceiveFocus = true;
		this.color = OptionalValue(o.color, "#ffffff");
		this.label = OptionalString(o.label, "Default");
		this.shadowBlur = OptionalNumber(o.shadowBlur, 8);

		this.shadowColor = OptionalValue(
			o.shadowColor, 
			"rgba(0, 0, 0, 0.5)"
		);

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

		this.handle = this.add("UIView", {
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
			label : self.label
		});

		if (o.movable) {

			this.handle.addEventListener("mousedown", function(e){
				self._mdownhandleStarted = true;
				/*
				self.transformOrigin = {
					x : e.x,
					y : e.y
				};
				self.set("scale", 1.10, 60);
				self.set("shadowBlur", 25, 70);
				self.shadowColor = "rgba(0, 0, 0, 0.98)";
				*/
			}, false);

			this.handle.addEventListener("dragstart", function(e){
			}, false);

			this.handle.addEventListener("drag", function(e){
				self.left += e.xrel;
				self.top += e.yrel;
			});

			this.handle.addEventListener("dragend", function(){
				self.unselect();
			}, false);

			this.handle.addEventListener("mouseup", function(){
				if (!self._mdownhandleStarted) return false;
				self._mdownhandleStarted = false;
				self.unselect();
			}, false);
		}

		if (o.closeable) {
			this.handle.closeButton = this.add("UIButtonClose", {
				left : this.width-19,
				top : 4,
				width : 16,
				height : 16,
				background : "rgba(0, 0, 0, 0.75)",
				color : "#888888"
			});

			this.handle.closeButton.addEventListener("mouseup", function(e){
				self.set("scale", 0, 120, function(){});
				self.shadowBlur = 0;
				self.shadowColor = "rgba(0, 0, 0, 0.20)";
				e.stopPropagation();
			}, false);
		}

		this.contentView = this.add("UIView", {
			left : 3,
			top : 24,
			width : self.width-6,
			height : self.height-27,
			radius : 4,
			background : "#ffffff",
			color : "#333333"
		});

		if (o.resizable) {
			this.resizer = this.add("UIWindowResizer");
		}

	},

	draw : function(context){
		var	params = this.getDrawingBounds(),
			radius = Math.max(4, this.radius);

		context.setShadow(0, 8, this.shadowBlur, this.shadowColor);

		context.roundbox(
			params.x, params.y, 
			params.w, params.h, 
			radius, this.background, false
		);

		context.setShadow(0, 0, 0);

		var gradient = context.createLinearGradient(
			params.x, params.y, 
			params.x, params.y+24
		);

		gradient.addColorStop(0.00, 'rgba(255, 255, 255, 0.20)');
		gradient.addColorStop(0.10, 'rgba(255, 255, 255, 0.05)');
		gradient.addColorStop(0.90, 'rgba(255, 255, 255, 0.00)');

		context.roundbox(
			params.x, params.y, 
			params.w, params.h, 
			radius, gradient, false
		);
	}
});

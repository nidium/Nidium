/* ------------------------+------------- */
/* Native Framework 2.0    | Falcon Build */
/* ------------------------+------------- */
/* (c) 2013 nidium.com - Vincent Fontaine */
/* -------------------------------------- */

Native.elements.export("UIWindow", {
	public : {
		width : {
			set : function(value){
				if (this.width<22) this.width = 22;
				this.handle.width = this.width;
				this.handle.closeButton.left = this.handle.width - 19;
				this.contentView.width = this.width - 6;
				this.resizer.updateElement();
				this.contentView.refreshScrollBars();
			}
		},

		height : {
			set : function(value){
				if (this.height<40) this.height = 40;
				this.contentView.height = this.height - 27;
				this.resizer.updateElement();
				this.contentView.refreshScrollBars();
			}
		}
	},


	init : function(){
		var self = this,
			o = this.options;

		this.setProperties({
			canReceiveFocus : true,
			color : OptionalValue(o.color, "#ffffff"),
			label : OptionalString(o.label, "Default"),
			shadowBlur : OptionalNumber(o.shadowBlur, 8),
			shadowColor : OptionalValue(o.shadowColor, "rgba(0, 0, 0, 0.5)"),
			radius : Math.max(5, OptionalNumber(o.radius, 5)),

			movable : OptionalBoolean(o.movable, true),
			resizable : OptionalBoolean(o.resizable, true),
			closeable : OptionalBoolean(o.closable, true)
		});

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
			label : self.label
		});

		this.labelElement.addEventListener("dragstart", function(e){
			e.forcePropagation();
		}, true);

		if (o.movable) {
			this.handle.addEventListener("mousedown", function(e){
				self._mdownhandleStarted = true;
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

		this.handle.closeButton = this.add("UIButtonClose", {
			left : this.width-19,
			top : 4,
			width : 16,
			height : 16,
			background : "rgba(0, 0, 0, 0.75)",
			color : "#888888"
		});

		if (o.closeable) {
			this.handle.closeButton.addEventListener("mouseup", function(e){
				//self.set("scale", 0, 120, function(){});
				self.fadeOut(150, function(){
					this.hide();
				});
				self.shadowBlur = 0;
				self.shadowColor = "rgba(0, 0, 0, 0.20)";
				e.stopPropagation();
			}, false);
		} else {
			this.handle.closeButton.hide();
		}

		this.contentView = this.add("UIView", {
			left : 3,
			top : 24,
			width : self.width-6,
			height : self.height-27,
			radius : 4,
			background : "#ffffff",
			color : "#333333",
			overflow : false,
			scrollable : true
		});

		if (o.resizable) {
			this.resizer = this.add("UIWindowResizer");
		}

	},

	draw : function(context){
		var	params = this.getDrawingBounds();

 		if (this.outlineColor && this.outline) {
			NDMElement.draw.outline(this);
		}

		context.setShadow(
			this.shadowOffsetX, this.shadowOffsetY,
			this.shadowBlur, this.shadowColor
		);
		NDMElement.draw.box(this, context, params);
		context.setShadow(0, 0, 0);

		var gradient = NDMElement.draw.getCleanGradient(this, context, params);
		NDMElement.draw.box(this, context, params, gradient);
	}
});

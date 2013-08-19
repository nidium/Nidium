/* ------------------------+------------- */
/* Native Framework 2.0    | Falcon Build */
/* ------------------------+------------- */
/* (c) 2013 Stight.com - Vincent Fontaine */
/* -------------------------------------- */

Native.elements.export("UIDropDownController", {
	public : {
		value : {
			set : function(value){
				this.setValue(value);
			},

			get : function(){
				return this.tabs[this.selection].value;
			}
		}
	},

	init : function(){
		var self = this,
			o = this.options;

		var y = 0,
			tabs = o.elements ? o.elements : [],
			l = tabs.length;

		this.setProperties({
			canReceiveFocus	: true,

			canReceiveKeyboardEvents : OptionalBoolean(
				o.canReceiveKeyboardEvents,
				true
			),

			label			: OptionalString(o.label, "Choose"),
			fontSize  		: OptionalNumber(o.fontSize, 11),
			fontType  		: OptionalString(o.fontType, "arial"),

			paddingLeft		: OptionalNumber(o.paddingLeft, 10),
			paddingRight	: OptionalNumber(o.paddingLeft, 10),

			width 			: OptionalNumber(o.width, 140),
			height 			: OptionalNumber(o.height, 22),
			maxHeight 		: OptionalNumber(o.maxHeight, null),
			radius 			: OptionalNumber(o.radius, 2),
			background 		: OptionalValue(o.background, "#2277E0"),
			color 			: OptionalValue(o.color, "#ffffff"),

			selectedBackground : OptionalValue(o.selectedBackground, "#4D90FE"),
			selectedColor : OptionalValue(o.selectedColor, "#ffffff"),

			name : OptionalString(o.name, "Default")
		});

		this.selection = 0;
		this.tabs = [];

		this.getSelectorHeight = function(){
			var	l = this.tabs.length,
				h = l*this.height;

			return this.maxHeight || h;
		};

		this.centerToSelection = function(duration){
			if (!this.tabs[this.selection]) return this;
			var tab = this.tabs[this.selection];
				y = tab.top,
				h = tab.height,
				sh = this.getSelectorHeight(),
				min = this.selector.scrollTop;

				pos = y - sh/2 + h/2;

			if (duration) {
				this.selector.animate(
					"scrollTop", min, pos, duration, function(){
					}, Math.physics.quintOut
				);
			} else {
				this.selector.scrollTop = pos;
			}

			return this;
		};


		this.scrollToSelection = function(){
			if (!this.tabs[this.selection]) return this;
			var tab = this.tabs[this.selection];
				y = tab.top,
				h = tab.height,
				sh = this.getSelectorHeight(),
				min = this.selector.scrollTop,
				max = min + sh;

			if (y < min) {
				this.selector.scrollTop = y;
			}

			if (y+h >= max) {
				this.selector.scrollTop = y + h - (max-min);
			}
	
			return this;
		};

		this.selectIndex = function(index){
			index = Math.max(Math.min(index, this.tabs.length-1), 0);
			if (this.selection != index){
				this.selection = index;
				this.reset(this.selection);
				this.fireEvent("change", {
					index : this.selection,
					value : this.tabs[this.selection].value
				});
			}
			return this;
		};

		this.setValue = function(value){
			for (var i=0; i<this.tabs.length; i++){
				if (this.tabs[i].value == value) {
					this.selectIndex(i);
					continue;
				}
			}
			return this;
		};

		this._addElement = function(i, options, y){
			var o = options,
				label = OptionalString(o.label, "Default"),
				selected = OptionalBoolean(o.selected, false),
				background = OptionalValue(o.background, "rgba(255, 255, 255, 1)"),
				className = OptionalString(o.class, ""),
				value = OptionalValue(o.value, ""),
				color = OptionalValue(o.color, "#888888"),
				selected = OptionalBoolean(o.selected, false),
				disabled = OptionalBoolean(o.disabled, false);

			if (selected) {
				self.selection = i;
			}

			this.selector.__unlock();
			this.tabs[i] = this.selector.add("UIOption", {
				left : 0,
				top : y,
				height : this.height,
				name : "option_" + this.name,
				label : label,
				class : className,
				selected : selected,
				disabled : disabled,
				background : background,
				color : color,
				value : value
			});

			this.tabs[i].index = i;
		};

		this.selector = this.add("UIView", {
			left : 2,
			top : this.height + 2,
			width : this.width - 4,
			height : 0,
			radius : 0,
			background : this.background,
			borderWidth : 1,
			borderColor : "rgba(0, 0, 0, 0.05)",
			shadowBlur : 4,
			shadowOffsetY : 2,
			shadowColor : "rgba(0, 0, 0, 0.15)",
			scrollable : true,
			overflow : false
		});

		for (var i=0; i<l; i++){
			self._addElement(i, tabs[i], y);
			y += this.tabs[i].height;
		}

		this.downButton = this.add("UIButtonDown", {
			left : this.width-this.height,
			top : 3,
			width : this.height-6,
			height : this.height-6,
			background : "rgba(0, 0, 0, 0.45)",
			color : "#ffffff"
		});

		this._showElements = function(){
			var l = tabs.length;
			for (var i=0; i<l; i++){
				self.tabs[i].show();
			}
		};

		this._hideElements = function(){
			var l = tabs.length;
			for (var i=0; i<l; i++){
				self.tabs[i].hide();
			}
		};

		this.reset = function(){
			for (var i=0; i<this.tabs.length; i++){
				this.tabs[i].selected = false;
			}

			if (this.tabs[this.selection]) {
				this.tabs[this.selection].selected = true;
				this.label = this.tabs[this.selection].label;
			}
			return this;
		};

		this.initSelector = function(){
			this.selector.hide();
			this.selector._animating = false;
			this.selector.opacity = 0;
			this.selector.height = 0;
			this.toggleState = false;
		};

		this.openSelector = function(){
			if (this.toggleState == true) return false;
			var c = this.selector,
				from = c.height,
				l = this.tabs.length,
				delta = l*self.height;


			if (c._animating) return false;

			if (this.maxHeight != 0){
				delta = Math.min(this.maxHeight, delta);
			}

			this.toggleState = true;
			c._animating = true;
			c.show();
			
			self._showElements();

			c.finishCurrentAnimations("opacity");
			c.finishCurrentAnimations("height");

			this.downButton.animate(
				"angle",
				this.downButton.angle,
				180,
				200,
				null,
				Math.physics.quintOut
			);

			c.animate(
				"opacity", 0, 1, 250,
				function(){},
				Math.physics.cubicOut
			);

			this.centerToSelection(400);

			c.animate(
				"height", from, delta, 200,
				function(){
					this._animating = false;
				},
				Math.physics.quintOut
			);

			return this;
		};

		this.closeSelector = function(init){
			var c = this.selector,
				from = c.height,
				l = this.tabs.length,
				delta = 0;

			if (c._animating) {
				this.toggleState = false;
				c.finishCurrentAnimations("opacity");
				c.finishCurrentAnimations("height");
				c.opacity = 0;
				c.height = 0;
				c._animating = false;
				c.hide();

				this.downButton.animate(
					"angle",
					this.downButton.angle,
					0,
					150,
					null,
					Math.physics.quintOut
				);

				this._hideElements();
				Native.events.tick();
				return false;
			}

			this.toggleState = false;
			c._animating = true;
			c.show();

			c.finishCurrentAnimations("opacity");
			c.finishCurrentAnimations("height");

			this.downButton.animate(
				"angle",
				this.downButton.angle,
				0,
				200,
				null,
				Math.physics.quintIn
			);

			c.animate("opacity", 1, 0, 250, function(){
			}, Math.physics.cubicIn);
			
			c.animate("height", from, delta, 200, function(){
				this._animating = false;
				self._hideElements();
				this.hide();
				Native.events.tick();
			}, Math.physics.quintIn);

			return this;
		};

		DOMElement.listeners.addHovers(this);

		this.addEventListener("mousedown", function(e){
			if (this.toggleState) {
				this.closeSelector();
			} else {
				this.openSelector();
			}
			e.stopPropagation();
		}, false);


		this.addEventListener("blur", function(e){
			this.closeSelector();
		}, false);

		this.addEventListener("keydown", function(e){
			if (!self.hasFocus) return false;
			switch (e.keyCode) {
				case 27 : // ESC
					self.closeSelector();
					break;

				case 13 : // ENTER
				case 32 : // SPACE
				case 1073741912 : // SMALL ENTER
					self.closeSelector();
					break;


				case 1073741906 : // up
					self.selectIndex(self.selection-1);
					self.scrollToSelection();
					break;

				case 1073741905 : // down
					self.selectIndex(self.selection+1);
					self.scrollToSelection();
					break;

				default : break;
			}
		}, false);

		this.initSelector();
		this.reset();
	},

	draw : function(context){
		var	params = this.getDrawingBounds();

 		if (this.outlineColor && this.outline) {
			DOMElement.draw.outline(this);
		}

		if (__ENABLE_BUTTON_SHADOWS__) {
			if (this.selected){
				context.setShadow(0, 1, 0.75, "rgba(255, 255, 255, 0.08)");
			} else {
				context.setShadow(0, 2, 4, "rgba(0, 0, 0, 0.15)");
			}
		}
		
		DOMElement.draw.box(this, context, params);

		if (__ENABLE_BUTTON_SHADOWS__){
			context.setShadow(0, 0, 0);
		}

		DOMElement.draw.glassLayer(this, context, params);
		DOMElement.draw.label(this, context, params);
	}
});

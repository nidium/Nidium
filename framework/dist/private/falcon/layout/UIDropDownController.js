/* ------------------------+------------- */
/* Native Framework 2.0    | Falcon Build */
/* ------------------------+------------- */
/* (c) 2013 nidium.com - Vincent Fontaine */
/* -------------------------------------- */

/* -------------------------------------------------------------------------- */
/* NSS PROPERTIES                                                             */
/* -------------------------------------------------------------------------- */

document.nss.add({
	"UIDropDownController" : {
		width 			: 180,
		height 			: 22,
		maxHeight 		: 198,

		label			: "Choose",
		fontSize  		: 11,
		fontFamily  	: "arial",

		paddingLeft		: 10,
		paddingRight	: 10,

		shadowBlur 		: 4,
		shadowColor 	: "rgba(0, 0, 0, 0.15)",
		shadowOffsetX 	: 0,
		shadowOffsetY 	: 2,

		radius 			: 2,
		background 		: "#262722",
		color 			: "#ffffff",
		canReceiveFocus	: true,
		canReceiveKeyboardEvents : true
	},

	"UIDropDownController:selected" : {
		shadowBlur : 0.75,
		shadowColor : "rgba(255, 255, 255, 0.08)",
		shadowOffsetX : 0,
		shadowOffsetY : 1
	}
});

/* -------------------------------------------------------------------------- */
/* ELEMENT DEFINITION                                                         */
/* -------------------------------------------------------------------------- */

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

	onAddChildRequest : function(child){
		var accept = false;
		
		/* forward children to the selector (not the UIDropDownController) */
		if (child.type == "UIOption") {
			var currentTabIndex = this.tabs.length,
				top = 0;

			for (var i=0; i<this.tabs.length; i++){
				top += this.tabs[i].height;
			}

			var options = {
				label : child.label,
				className : child.className,
				value : child.value,
				selected : child.selected,
				disabled : child.disabled
			};

			this._addElement(currentTabIndex, options, top);
			return false;
		}

		if (child.type.in("UIView", "UIButtonDown")) accept = true;
		return accept;
	},

	init : function(){
		var self = this,
			o = this.options;

		var y = 0,
			tabs = o.elements ? o.elements : [];

		this.name = OptionalString(o.name, "Default");
		this.selection = 0;
		this.tabs = [];
		this.hideSelector = OptionalBoolean(o.hideSelector, false);
		this.hideToggleButton = OptionalBoolean(o.hideToggleButton, false);


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
			var old = this.selection;
			index = Math.max(Math.min(index, this.tabs.length-1), 0);

			this.selection = index;
			this.reset(this.selection);

			if (this.selection != index){
				this.fireEvent("change", {
					index : this.selection,
					value : this.tabs[this.selection].value
				});
			}

			this.fireEvent("select", {
				index : this.selection,
				value : this.tabs[this.selection].value
			});

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
				className = OptionalString(o.class, ""),
				value = OptionalValue(o.value, ""),
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
				value : value
			});

			this.tabs[i].index = i;

			this.tabs[i].addEventListener("mousedown", function(e){
				if (this.disabled) {
				} else {
					self.selectIndex(this.index);
					self.closeSelector();
				}
				e.stopPropagation();
			}, false);
		};

		this.selector = this.add("UIView", {
			left : this.hideSelector ? 0 : 2,
			top : this.hideSelector ? 0 : this.height + 2,
			width : this.hideSelector ? this.width : this.width - 4,
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

		for (var i=0; i<tabs.length; i++){
			self._addElement(i, tabs[i], y);
			y += this.tabs[i].height;
		}

		this.downButton = this.add("UIButtonDown", {
			left : this.width-this.height,
			top : 3,
			width : this.height-6,
			height : this.height-6,
			background : "rgba(0, 0, 0, 0.25)",
			color : "#ffffff"
		});
		if (this.hideToggleButton) this.downButton.hide();

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

		this.unselect = function(){
			for (var i=0; i<this.tabs.length; i++){
				this.tabs[i].selected = false;
			}
			return this;
		};

		this.reset = function(){
			this.unselect();
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

		this.toggleSelector = function(){
			if (this.toggleState){
				this.closeSelector();
			} else {
				this.openSelector();
			}
		};

		this.openSelector = function(duration=400){
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
			c.bringToFront().show();
			
			self._showElements();

			c.finishCurrentAnimations("opacity");
			c.finishCurrentAnimations("height");

			if (this.hideToggleButton === false) {
				this.downButton.animate(
					"angle",
					this.downButton.angle,
					180,
					duration>>1,
					null,
					Math.physics.quintOut
				);
			}

			if (duration) {
				c.animate(
					"opacity", 0, 1, 50+duration>>1,
					function(){},
					Math.physics.cubicOut
				);
			} else {
				c.opacity = 1;
			}

			this.centerToSelection(duration);

			if (duration) {
				c.animate(
					"height", from, delta, duration>>1,
					function(){
						this._animating = false;
					},
					Math.physics.quintOut
				);
			} else {
				c.height = delta;
				c._animating = false;
			}

			return this;
		};

		this.closeSelector = function(){
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

				if (this.hideToggleButton === false) {
					this.downButton.animate(
						"angle",
						this.downButton.angle,
						0,
						150,
						null,
						Math.physics.quintOut
					);
				}

				this._hideElements();
				window.events.tick();
				return false;
			}

			this.toggleState = false;
			c._animating = true;
			c.show();

			c.finishCurrentAnimations("opacity");
			c.finishCurrentAnimations("height");

			if (this.hideToggleButton === false) {
				this.downButton.animate(
					"angle",
					this.downButton.angle,
					0,
					200,
					null,
					Math.physics.quintIn
				);
			}

			c.animate("opacity", 1, 0, 250, function(){
			}, Math.physics.cubicIn);
			
			c.animate("height", from, delta, 200, function(){
				this._animating = false;
				self._hideElements();
				this.hide();
				window.events.tick();
			}, Math.physics.quintIn);

			return this;
		};

		NDMElement.listeners.addHovers(this);

		this.addEventListener("contextmenu", function(e){
			e.preventDefault();
		}, false);

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

		this.addEventListener("focus", function(e){
			this.bringToFront();
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
					self.toggleSelector();
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

 		if (this.hideSelector) return;

 		if (this.outlineColor && this.outline) {
			NDMElement.draw.outline(this);
		}

		NDMElement.draw.softShadow(this);
		NDMElement.draw.box(this, context, params);
		NDMElement.draw.disableShadow(this);

		NDMElement.draw.glassLayer(this, context, params);
		NDMElement.draw.label(this, context, params);
	}
});

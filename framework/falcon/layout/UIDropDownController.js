/* -------------------------- */
/* Native (@) 2012 Stight.com */
/* -------------------------- */

Native.elements.export("UIDropDownController", {
	init : function(){
		var self = this,
			o = this.options;

		var y = 0,
			tabs = o.elements ? o.elements : [],
			l = tabs.length;

		this.setProperties({
			canReceiveFocus	: true,
			label			: OptionalString(o.label, "Choose"),
			fontSize  		: OptionalNumber(o.fontSize, 11),
			fontType  		: OptionalString(o.fontType, "arial"),

			paddingLeft		: OptionalNumber(o.paddingLeft, 10),
			paddingRight	: OptionalNumber(o.paddingLeft, 10),

			width 			: OptionalNumber(o.width, 140),
			height 			: OptionalNumber(o.height, 22),
			radius 			: OptionalNumber(o.radius, 2),
			background 		: OptionalValue(o.background, "#2277E0"),
			color 			: OptionalValue(o.color, "#ffffff"),

			selectedBackground : OptionalValue(o.selectedBackground, "#4D90FE"),
			selectedColor : OptionalValue(o.selectedColor, "#ffffff"),

			name : OptionalString(o.name, "Default")
		});

		this.selection = 0;
		this.tabs = [];

		this.resetTabs = function(){
			var l = this.tabs.length;
			for (var i=0; i<l; i++){
				this.tabs[i].selected = false;
			}

			if (this.tabs[this.selection]) {
				this.tabs[this.selection].selected = true;
			}
		};

		this.selectTab = function(tabnum){
			this.selection = Math.max(Math.min(tabnum, this.tabs.length-1), 0);
			this.label = this.tabs[this.selection].label;
			this.resetTabs(this.selection);
			this.closeSelector();
		};

		this.selectNextTab = function(){
		};

		this.selectPreviousTab = function(){
		};

		this.removeTab = function(position){
		};

		this.insertTab = function(position, options){
		};

		this._addTab = function(i, options, y){
			var o = options,
				label = OptionalString(o.label, "Default"),
				selected = OptionalBoolean(o.selected, false),
				background = OptionalValue(o.background, "#262722"),
				color = OptionalValue(o.color, "#abacaa"),
				selected = OptionalBoolean(o.selected, false);

			if (selected) {
				self.selection = i;
			}

			this.tabs[i] = this.tabsContainer.add("UIDropDownOption", {
				left : 40,
				top : y,
				height : this.height,
				name : "option_" + this.name,
				label : label,
				selected : selected,
				background : background,
				color : color
			});

			this.tabs[i].tabnum = i;
		};

		this.tabsContainer = this.add("UIView", {
			left : 2,
			top : this.height+2,
			width : this.width-4,
			height : 250,
			radius : 0,
			background : '#FF2722',
			shadowBlur : 10
		});

		for (var i=0; i<l; i++){
			self._addTab(i, tabs[i], y);
			y += this.tabs[i].height;
			//this.tabs[i].visible = false;
		}

		this.downButton = this.add("UIButtonDown", {
			left : this.width-this.height,
			top : 3,
			width : this.height-6,
			height : this.height-6,
			background : "rgba(0, 0, 0, 0.45)",
			color : "#ffffff"
		});

		this.showTabs = function(){
			var l = tabs.length;
			for (var i=0; i<l; i++){
				self.tabs[i].visible = true;
//				self.tabs[i].draw();
			}
		};

		this.hideTabs = function(){
			var l = tabs.length;
			for (var i=0; i<l; i++){
				self.tabs[i].visible = false;
			}
		};

		this.openSelector = function(){
			if (this.toggleState == true) { return false; }
			var from = self.tabsContainer.height,
				delta = l*self.height;


			this.toggleState = true;
			
			self.showTabs();

			self.tabsContainer.animate("height", from, delta, 180, function(){
				self.showTabs();
			}, Math.physics.cubicOut);

		};

		this.closeSelector = function(){
			if (this.toggleState == false) { return false; }
			var from = self.tabsContainer.height,
				delta = 0;

			this.toggleState = false;
			
			self.tabsContainer.animate("height", from, delta, 150, function(){
				self.hideTabs();
			}, Math.physics.quintOut);

		};

		DOMElement.listeners.addHovers(this);

		this.addEventListener("mousedown", function(e){
			if (this.toggleState) {
				this.closeSelector();
			} else {
				this.openSelector();
			}
			//this.bringToTop();
			e.stopPropagation();
		}, false);


		this.addEventListener("blur", function(e){
			this.closeSelector();
		}, false);

		this.resetTabs();

		//this.tabsContainer.height = 1;
		//this.closeSelector();
	},

	draw : function(context){
		var	params = this.getDrawingBounds();

		if (__ENABLE_BUTTON_SHADOWS__) {
			if (this.selected){
				context.setShadow(0, 1, 0.75, "rgba(255, 255, 255, 0.08)");
			} else {
				context.setShadow(0, 2, 3, "rgba(0, 0, 0, 0.5)");
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
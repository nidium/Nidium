/* -------------------------- */
/* Native (@) 2012 Stight.com */
/* -------------------------- */

Native.elements.export("UIDropDownController", {
	init : function(){
		var self = this;

		this.flags._canReceiveFocus = true;

		this.w = OptionalNumber(this.options.w, 140);
		this.h = OptionalNumber(this.options.h, 24);
		
		this.background = OptionalValue(this.options.background, '#191a18');
		this.color = OptionalValue(this.options.color, "#ffffff");
		this.selectedBackground = OptionalValue(this.options.selectedBackground, "#4D90FE");
		this.selectedColor = OptionalValue(this.options.selectedColor, "#FFFFFF");
		this.name = OptionalString(this.options.name, "Default");
		this.selection = 0;
		this.tabs = [];

		this.resetTabs = function(){
			let	l = self.tabs.length;
			for (let i=0; i<l; i++){
				self.tabs[i].selected = false;
			}

			if (self.tabs[self.selection]) {
				self.tabs[self.selection].selected = true;
			}
		};

		this.selectTab = function(tabnum){
			self.selection = Math.max(Math.min(tabnum, self.tabs.length-1), 0);
			self.resetTabs(self.selection);
			self.closeSelector();
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
			var label = options.label ? options.label : "New options",
				selected = options.selected ? options.selected : false,
				background = options.background ? options.background : "#262722",
				color = options.color ? options.color : "#abacaa";

			if (selected) {
				self.selection = i;
			}

			this.tabs[i] = this.tabsContainer.add("UIDropDownOption", {
				x : 0,
				y : y,
				name : "option_" + this.name,
				label : label,
				selected : selected,
				background : background,
				color : color
			});

			this.tabs[i].tabnum = i;
			//this.tabs[i].zIndex = this.zIndex-1;
		};

		var y = 0,
			tabs = this.options.elements ? this.options.elements : [],
			l = tabs.length;

		this.tabsContainer = this.add("UIView", {
			x : 2,
			y : this.h+2,
			w : this.w-4,
			h : 0.001,
			radius : 0,
			background : '#262722',
			shadowBlur : 10
		});

		this.tabsContainer.zIndex += 1000;

		for (let i=0; i<l; i++){
			self._addTab(i, tabs[i], y);
			y += this.tabs[i].h;
			this.tabs[i].visible = false;
		}

		this.downButton = this.add("UIButtonDown", {
			x : this.w-18,
			y : 4,
			w : 16,
			h : 16,
			background : "rgba(0, 0, 0, 0.75)",
			color : "#888888"
		});

		this.showTabs = function(){
			let l = tabs.length;
			for (let i=0; i<l; i++){
				self.tabs[i].visible = true;
				self.tabs[i].draw();
			}
		};

		this.hideTabs = function(){
			let l = tabs.length;
			for (let i=0; i<l; i++){
				self.tabs[i].visible = false;
			}
		};

		this.openSelector = function(){
			if (this.toggleState == true) { return false; }
			var from = self.tabsContainer.h,
				delta = l*self.h;


			this.toggleState = true;
			
			self.showTabs();

			self.tabsContainer.animate("h", from, delta, 180, function(){
				self.showTabs();
			}, Math.physics.cubicOut);

		};

		this.closeSelector = function(){
			if (this.toggleState == false) { return false; }
			var from = self.tabsContainer.h,
				delta = 0;

			this.toggleState = false;
			
			self.tabsContainer.animate("h", from, delta, 150, function(){
				self.hideTabs();
			}, Math.physics.quintOut);

		};

		this.addEventListener("mousedown", function(e){
			if (this.toggleState) {
				self.closeSelector();
			} else {
				self.openSelector();
			}
			this.bringToTop();
			e.stopPropagation();
		}, false);


		this.addEventListener("blur", function(e){
			self.closeSelector();
		}, false);

		this.resetTabs();

		this.tabsContainer.h = 0;
		this.closeSelector();
	},

	draw : function(){
		var params = {
				x : this._x,
				y : this._y,
				w : this.w,
				h : this.h
			},
	
			radius = Math.max(4, this.radius),
			textHeight = 10,
			textOffsetX = 8,
			textOffsetY = (this.h-textHeight)/2 + 9,
			textColor = this.color,
			textShadow = '#000000',

			label = this.tabs[this.selection].label;



		this.shadow = true;
		if (this.shadow) {
			if (this.selected){
				canvas.setShadow(0, 2, 1, "rgba(255, 255, 255, 0.05)");
			} else {
				canvas.setShadow(0, 2, 3, "rgba(0, 0, 0, 0.5)");
			}
		}
		canvas.roundbox(params.x, params.y, params.w, params.h, radius, this.background, false); // main view
		if (this.shadow){
			canvas.setShadow(0, 0, 0);
		}

		var gdBackground = canvas.createLinearGradient(params.x, params.y, params.x, params.y+params.h);
		gdBackground.addColorStop(0.00, 'rgba(255, 255, 255, 0.20)');
		gdBackground.addColorStop(0.10, 'rgba(255, 255, 255, 0.05)');
		gdBackground.addColorStop(0.90, 'rgba(255, 255, 255, 0.00)');

		canvas.roundbox(params.x, params.y, params.w, params.h, radius, gdBackground, false); // main view

		canvas.setFontSize(11);

		//if (this.shadow) { canvas.setShadow(1, 1, 1, textShadow); }
		canvas.setColor(textColor);
		canvas.fillText(label, params.x+textOffsetX, params.y+textOffsetY);
		//if (this.shadow){ canvas.setShadow(0, 0, 0); }



	}
});
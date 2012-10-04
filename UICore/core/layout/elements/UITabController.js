/* -------------------------- */
/* Native (@) 2012 Stight.com */
/* -------------------------- */

Native.elements.export("UITabController", {
	init : function(){
		var self = this;

		this.w = this.parent.w;
		this.h = 32;
		this.background = this.options.background || "";
		this.name = this.options.name || "Default";
		this.overlap = 12;
		this.selection = false;
		this.position = false;

		this.tabs = [];
		this.taborder = [];

		this.resetTabs = function(animate, callback){
			let x = 0,
				j = 2,
				l = self.taborder.length;

			for (let i=0; i<l; i++){
				let t = self.taborder[i];
				self.tabs[t].position = i;
				self.tabs[t].zIndex = l-i-1;
				self.tabs[t].selected = false;
				if (animate) {
					var _callback = (i==1 ? callback : null);
					self.tabs[t].slideX(x, 26*j++, _callback);
				} else {
					self.tabs[t].left = x;
				}
				x += self.tabs[t].w-this.overlap;
			}
			if (self.tabs[self.selection]) {
				self.tabs[self.selection].selected = true;
				self.tabs[self.selection].zIndex = 2*l;
			}
		};

		this.selectTab = function(tabnum){
			self.selection = Math.max(Math.min(tabnum, self.tabs.length-1), 0);
			self.resetTabs();
			self.position = self.getPosition(self.selection);

			self.fireEvent("tabselect", {
				tab : self.selection,
				position : self.position
			});
		};

		this.selectNextTab = function(){
			var p = self.position, // current selected tab position
				tab = self.getTab(p+1) ? self.getTab(p+1).tabnum : self.getTab(p).tabnum; // next tab at position p+1

			self.selectTab(tab);
		};

		this.selectPreviousTab = function(){
			var p = self.position, // current selected tab position
				tab = self.getTab(p-1) ? self.getTab(p-1).tabnum : self.getTab(p).tabnum;; // previous tab at position p-1

			self.selectTab(tab);
		};

		this.removeTab = function(position){
			let tab = self.getTab(position);
			if (!tab) {
				return false;
			}

			tab.g = {
				x : 0,
				y : tab.h/2
			};
			tab.bounceScale(0, 120, function(){
				self._removeTab(this.tabnum);
				//this.remove();
			});

		};

		this._removeTab = function(tabnum){
			let to = self.taborder,
				position = self.tabs[tabnum].position,
				tb = self.tabs;


			for (let i=0; i<to.length; i++){
				if (to[i] > tabnum){
					to[i]--;
				}
			}

			to.splice(position, 1);

			for (let i=tabnum; i<tb.length; i++){
				tb[i].position--;
				tb[i].tabnum--;
			}

			tb.splice(tabnum, 1);
			
			self.resetTabs(true);

			self.fireEvent("tabclose", {
				tab : tabnum,
				positions : to
			});
		};


		this.insertTab = function(position, options){
			var to = self.taborder,
				tb = self.tabs,
				tab = self.getTab(position),
				tabnum = tab.tabnum,
				left = tab.left || false,
				newtab;

			for (var i=tabnum; i<tb.length; i++){
				tb[i].position++;
				tb[i].tabnum++;
			}
			tb.splice(tabnum, 0, {});

			self._addTab(tabnum, position, options);

			newtab = tb[tabnum];
			newtab.left = left;
			newtab.g = {
				x : 0,
				y : newtab.h/2
			};
			newtab.scale = 0;

			for (var i=0; i<to.length; i++){
				if (to[i] >= tabnum){
					to[i]++;
				}
			}
			to.splice(position, 0, tabnum);
			
			self.resetTabs(true, function(){
				newtab.bounceScale(1, 80, function(){
					//self._removeTab(this.tabnum);
					//this.remove();
				});
			});

			self.fireEvent("tabinsert", {
				tab : tabnum,
				positions : to
			});
		};


		this.swapTabs = function(x, y){
			let A = self.taborder;
			A[x]= A.splice(y, 1, A[x])[0];

			self.tabs[A[x]].position = x;
			self.tabs[A[y]].position = y;
		};

		this.getPosition = function(tabnum){
			let r = false,
				to = self.taborder;

			for (let i=0; i<to.length; i++){
				if (to[i] == tabnum) {
					r = i;
					break;
				}
			}
			return r;
		};

		this.getTab = function(position){
			return self.tabs[ self.taborder[position] ];
		};


		this._addTab = function(i, position, options, x){
			let label = options.label ? options.label : "New tab",
				selected = options.selected ? options.selected : false,
				closable = options.closable===false ? false : true,
				preventmove = options.preventmove===true ? true : false,
				background = options.background ? options.background : "#262722",
				color = options.color ? options.color : "#abacaa",

				l = tabs.length;


			if (selected) {
				self.selection = i;
				self.position = i;
			}
			
			this.tabs[i] = this.add("UITab", {
				x : x,
				y : 8,
				name : "tab_" + this.name,
				label : label,
				selected : selected,
				background : background,
				color : color,
				closable : closable,
				preventmove : preventmove
			});

			this.tabs[i].tabnum = i;
			this.tabs[i].position = position;
			this.tabs[i].zIndex = l-i-1;
		};

		var x = 0,
			tabs = this.options.tabs ? this.options.tabs : [],
			l = 0;

			/*
			tabs.push({
				label : "+",
				closable : false,
				preventmove : true,
				type : "normal"
			});
			*/

			l = tabs.length;

		for (let i=0; i<l; i++){
			self._addTab(i, i, tabs[i], x);
			self.taborder[i] = i;
			x += self.tabs[i].w-self.overlap;
		}

		self.resetTabs();

	},

	draw : function(){
		var params = {
				x : this._x,
				y : this._y,
				w : this.w,
				h : this.h
			};

		canvas.roundbox(params.x, params.y, params.w, params.h, this.radius, this.background, false); // main view

	}
});
/* -------------------------- */
/* Native (@) 2012 Stight.com */
/* -------------------------- */

Native.elements.export("UITabController", {
	refresh : function(){
		this.width = this.parent.width;
		this.height = 32;
	},

	init : function(){
		var self = this;

		this.background = OptionalValue(this.options.background, "");
		this.name = OptionalString(this.options.name, "Default");
		this.overlap = OptionalNumber(this.options.overlap, 14);
		this.selection = false;
		this.position = false;

		this.tabs = [];
		this.taborder = [];

		this.resetTabs = function(animate, callback){
			var x = 0,
				j = 2,
				nbtabs = self.taborder.length,
				selectedTab = self.tabs[self.selection];

			for (var i=0; i<nbtabs; i++){
				var t = self.taborder[i];
				self.tabs[t].position = i;
				self.tabs[t].selected = false;
				self.tabs[t].sendToBack();
				if (animate) {
					var _callback = (i==1 ? callback : null);
					self.tabs[t].slideX(x, 26*j++, _callback);
				} else {
					self.tabs[t].left = x;
				}
				x += self.tabs[t].width-this.overlap;
			}

			if (selectedTab) {
				selectedTab.selected = true;
				selectedTab.bringToFront();
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
				
				// next tab at position p+1
				tab = self.getTab(p+1) ? 
					  self.getTab(p+1).tabnum : self.getTab(p).tabnum;

			self.selectTab(tab);
		};

		this.selectPreviousTab = function(){
			var p = self.position, // current selected tab position
			
				// previous tab at position p-1
				tab = self.getTab(p-1) ? 
					  self.getTab(p-1).tabnum : self.getTab(p).tabnum; 

			self.selectTab(tab);
		};

		this.removeTab = function(position){
			var tab = self.getTab(position);
			if (!tab) {
				return false;
			}

			/*
			tab.g = {
				x : 0,
				y : tab.h/2
			};
			tab.bounceScale(0, 120, function(){
				self._removeTab(this.tabnum);
				//this.remove();
			});
			*/
			tab.hide();
			self._removeTab(this.tabnum);

		};

		this._removeTab = function(tabnum){
			var to = self.taborder,
				position = self.tabs[tabnum].position,
				tb = self.tabs;


			for (var i=0; i<to.length; i++){
				if (to[i] > tabnum){
					to[i]--;
				}
			}

			to.splice(position, 1);

			for (var i=tabnum; i<tb.length; i++){
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

			/*
			newtab.g = {
				x : 0,
				y : newtab.h/2
			};
			newtab.scale = 0;
			*/
			newtab.hide();

			for (var i=0; i<to.length; i++){
				if (to[i] >= tabnum){
					to[i]++;
				}
			}
			to.splice(position, 0, tabnum);
			
			self.resetTabs(true, function(){
				/*
				newtab.bounceScale(1, 80, function(){
					//self._removeTab(this.tabnum);
					//this.remove();
				});
				*/
				newtab.show();
			});

			self.fireEvent("tabinsert", {
				tab : tabnum,
				positions : to
			});
		};


		this.swapTabs = function(x, y){
			var A = self.taborder;
			A[x]= A.splice(y, 1, A[x])[0];

			self.tabs[A[x]].position = x;
			self.tabs[A[y]].position = y;
		};

		this.getPosition = function(tabnum){
			var r = false,
				to = self.taborder;

			for (var i=0; i<to.length; i++){
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
			var o = options,
				selected = OptionalBoolean(o.selected, false),
				l = tabs.length;

			if (selected) {
				self.selection = i;
				self.position = i;
			}
			
			this.tabs[i] = this.add("UITab", {
				left : x,
				top : 8,
				name : "tab_" + this.name,
				selected : selected,

				label : OptionalString(o.label, "New Tab"),
				background : OptionalValue(o.background, "#262722"),
				color : OptionalValue(o.color, "#abacaa"),
				closable : OptionalBoolean(o.closable, true),
				preventmove : OptionalBoolean(o.preventmove, false)
			});

			this.tabs[i].tabnum = i;
			this.tabs[i].position = position;
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


		for (var i=0; i<l; i++){
			this._addTab(i, i, tabs[i], x);
			this.taborder[i] = i;
			x += this.tabs[i].width - this.overlap;
		}
		this.resetTabs();
	},

	draw : function(context){
		var	params = this.getDrawingBounds();

		context.roundbox(
			params.x, params.y, 
			params.w, params.h, 
			this.radius, this.background, false
		);

	}
});
/* ------------------------+------------- */
/* Native Framework 2.0    | Falcon Build */
/* ------------------------+------------- */
/* (c) 2013 Stight.com - Vincent Fontaine */
/* -------------------------------------- */

Native.elements.export("UITabController", {
	public : {
		overlap : {
			set : function(value){
				this.resetTabs();
			}
		}
	},

	init : function(){
		var self = this,
			o = this.options,
			controller = this;

		this.overlap = OptionalValue(o.overlap, 14);
		this.background = OptionalValue(o.background, "");
		this.name = OptionalString(o.name, "Default");
		this.width = OptionalNumber(o.width, this.parent.width);
		this.height = OptionalNumber(o.height, 32);

		this.selection = false;
		this.position = false;

		this.tabs = [];
		this.taborder = [];

		this.resetTabs = function(animate, callback){
			var x = 0,
				j = 2,
				nbtabs = this.taborder.length,
				selelectedTab = this.getSelectedTab();

			for (var i=0; i<nbtabs; i++){
				var t = this.taborder[i],
					tab = this.tabs[t];

				tab.position = i;
				tab.unselect();

				if (animate) {
					var _callback = (i==1 ? callback : null);
					tab.slideX(x, 30*j++, _callback);
				} else {
					tab.left = x;
				}
				x += tab._width - this.overlap;
			}

			if (selelectedTab) {
				selelectedTab.select();
			}

			return selelectedTab;
		};

		this.selectTabAtIndex = function(index){
			if (self.selection == index) return false;
			self.selection = Math.max(Math.min(index, self.tabs.length-1), 0);
			self.position = self.getPosition(self.selection);

			var selectedTab = this.resetTabs();
			
			self.fireEvent("tabselect", {
				tab : selectedTab,
				index : self.selection,
				position : self.position
			});
			return selectedTab;
		};

		this.selectTabAtPosition = function(position){
			var tab = this.getTabAtPosition(position);
			if (!tab) {
				return false;
			} else if (self.position == position) {
				return tab;
			} else {
				return this.selectTabAtIndex(tab.index);
			}
		};

		this.getSelectedTab = function(){
			return self.tabs[self.selection];
		};

		this.selectTab = function(tab){
			if (!tab) return false;
			return this.selectTabAtIndex(tab.index);
		};

		this.selectNextTab = function(){
			var p = self.position, // current position of selected tab
				
				// next tab at position p+1
				tab = self.getTabAtPosition(p+1) ? 
					  self.getTabAtPosition(p+1) : self.getTabAtPosition(p);

			self.selectTab(tab);
		};

		this.selectPreviousTab = function(){
			var p = self.position, // current selected tab position
			
				// previous tab at position p-1
				tab = self.getTabAtPosition(p-1) ? 
					  self.getTabAtPosition(p-1) : self.getTabAtPosition(p);

			self.selectTab(tab);
		};

		this.removeTabAtPosition = function(position){
			var tab = self.getTabAtPosition(position);
			self._removeTabElement(tab);
		};

		this.removeTab = function(tab){
			if (!tab || this.__removing || this.__removeStarted) return false;
			self.__removeStarted = true;
			tab.fadeOut(120, function(){
				self.__removeStarted = false;
				controller._removeTabElement(this);
			});
		};

		this._removeTabElement = function(tab){
			if (!tab || this.__removing) return false;

			var index = tab.index,
				to = self.taborder,
				position = self.tabs[index] ? self.tabs[index].position : null,
				tb = self.tabs;

			if (position == null) return false;

			this.__removing = true;

			tab.__lock("_removeTabElement");
			tab.width = 0;
			tab.height = 0;
			tab.remove();
			tab.__unlock("_removeTabElement");

			for (var i=0; i<to.length; i++){
				if (to[i] > index){
					to[i]--;
				}
			}

			to.splice(position, 1);

			for (var i=index; i<tb.length; i++){
				tb[i].position--;
				tb[i].index--;
			}

			tb.splice(index, 1);
			
			self.resetTabs(true);

			self.fireEvent("tabclose", {
				index : index,
				positions : to
			});
			this.__removing = false;
		};

		this.insertTab = function(position, options){
			var to = self.taborder,
				tb = self.tabs,
				tab = self.getTabAtPosition(position),
				index = tab ? tab.index : 0,
				left = tab && tab.left ? tab.left : 0,
				newtab;

			for (var i=index; i<tb.length; i++){
				tb[i].position++;
				tb[i].index++;
			}
			tb.splice(index, 0, {});

			self._addTab(index, position, options);

			newtab = tb[index];

			newtab.__lock("insertTab");
			newtab.left = left;
			newtab.__unlock("insertTab");

			newtab.opacity = 0;

			for (var i=0; i<to.length; i++){
				if (to[i] >= index){
					to[i]++;
				}
			}
			to.splice(position, 0, index);
			
			self.resetTabs(true, function(){
				newtab.fadeIn(150);
			});

			self.fireEvent("tabinsert", {
				index : index,
				positions : to
			});
		};

		this.swapTabs = function(pos1, pos2){
			var A = self.taborder;
			A[pos1]= A.splice(pos2, 1, A[pos1])[0];

			self.tabs[A[pos1]].position = pos1;
			self.tabs[A[pos2]].position = pos2;
		};

		this.getPosition = function(index){
			var r = false,
				to = self.taborder;

			for (var i=0; i<to.length; i++){
				if (to[i] == index) {
					r = i;
					break;
				}
			}
			return r;
		};

		this.getTabAtPosition = function(position){
			return self.tabs[ self.taborder[position] ];
		};

		this._addTab = function(i, position, options, left){
			var o = options,
				selected = OptionalBoolean(o.selected, false),
				l = tabs.length;

			if (selected) {
				self.selection = i;
				self.position = i;
			}
			
			this.tabs[i] = this.add("UITab", {
				left : left,
				top : 8,
				name : "tab_" + this.name,
				height : this.height - 8,
				selected : selected,

				label : OptionalString(o.label, "New Tab"),
				background : OptionalValue(o.background, "#262722"),
				color : OptionalValue(o.color, "#abacaa"),
				opacity : OptionalNumber(o.opacity, 1),
				fontSize : OptionalNumber(o.fontSize, 11),
				fontType : OptionalString(o.fontType, "arial"),
				closable : OptionalBoolean(o.closable, true),
				preventmove : OptionalBoolean(o.preventmove, false),
				target : OptionalValue(o.target, null)
			});

			this.tabs[i].index = i;
			this.tabs[i].position = position;

			this.attachListenersToTab(this.tabs[i]);
		};

		this._slideTab = function(tab, value){
			tab.slideX(value, 180, null, Math.physics.cubicOut);
		};

		var __fireEvent = false,
			__startX = 0,
			__endX = 0,
			__dragTabPosition = false;

		this.attachListenersToTab = function(tab){
			tab.addEventListener("mousedown", function(e){
				controller.selectTab(this);
				e.stopPropagation();
			}, false);

			tab.addEventListener("dragstart", function(e){
				__fireEvent = false;
				__startX = tab.left;
				__endX = tab.left + tab.width;
				__dragTabPosition = controller.getPosition(tab.index);
			}, false);

			tab.addEventListener("dragend", function(e){
				if (__dragTabPosition===false) return false;
				
				var curr = controller.getTabAtPosition(__dragTabPosition);

				curr.slideX(__startX, 200, function(){}, Math.physics.cubicOut);

				__dragTabPosition = false;
				__startX = false;
				__endX = false;

				if (__fireEvent){
					controller.fireEvent("tabmove", {
						index : tab.index,
						positions : controller.taborder
					});
					__fireEvent = false;
				}
			}, false);

			if (tab.closeButton){
				tab.closeButton.addEventListener("mouseup", function(){
					if (this.hover){
						controller.removeTab(tab);
					}
				}, false);
			}
		};

		document.addEventListener("dragover", function(e){
			if (__dragTabPosition===false) return false;

			var i = __dragTabPosition,
				curr = controller.getTabAtPosition(i),
				next = controller.getTabAtPosition(i+1),
				prev = controller.getTabAtPosition(i-1),

				cx = curr.__left,
				cw = curr.width,

				nx = next ? next.__left : null,
				px = prev ? prev.__left : null;

			if (cx + e.xrel < controller.__left) {
				curr.left = controller.left;
			} else if (cx + cw + e.xrel > controller.__left + controller.width){
				curr.left = controller.width - cw;
			} else {
				curr.left += e.xrel;
			}

			if (next && e.xrel>0 && cx+cw > (nx+next.width/2)){
				controller.position = __dragTabPosition+1;
				controller.fireEvent("tabswap", {
					index : controller.selection,
					position : __dragTabPosition+1
				});

				controller._slideTab(next, __startX);
				controller.swapTabs(i, i+1);

				__fireEvent = true;

				__dragTabPosition++;

				i = __dragTabPosition;
				curr = controller.getTabAtPosition(i);
				next = controller.getTabAtPosition(i+1);
				prev = controller.getTabAtPosition(i-1);

				__startX += prev.width - controller.overlap;
				__endX += prev.width - controller.overlap;
			}

			if (prev && e.xrel<0 && cx < (px+prev.width/2)){
				controller.position = __dragTabPosition-1;
				controller.fireEvent("tabswap", {
					index : controller.selection,
					position : __dragTabPosition-1
				});

				controller._slideTab(prev, __endX - prev.width);
				controller.swapTabs(i, i-1);

				__fireEvent = true;

				__dragTabPosition--;

				i = __dragTabPosition;
				curr = controller.getTabAtPosition(i);
				next = controller.getTabAtPosition(i+1);
				prev = controller.getTabAtPosition(i-1);

				__startX -= next.width - controller.overlap;
				__endX -= next.width - controller.overlap;
			}
		}, false);

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
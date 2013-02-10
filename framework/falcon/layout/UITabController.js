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

		this.currentIndex = false;
		this.currentPosition = false;

		this.tabs = [];
		this.taborder = [];

		this.resetTabs = function(animate, callback){
			var that = this,
				counter = 0,
				x = 0,
				j = 2,
				nbtabs = this.taborder.length,
				selelectedTab = this.getSelectedTab();

			var sync = function(){
				counter++;
				if (counter == 1) {
					if (typeof callback == "function") callback.call(this);
				}
				Native.events.tick();
			};

			for (var i=0; i<nbtabs; i++){
				var t = this.taborder[i],
					tab = this.tabs[t];

				tab.pos = i;
				tab.unselect();

				if (animate) {
					//var _callback = (i==1 ? callback : null);
					//tab.slideX(x, 30*j++, _callback);
					tab.slideX(x, 30*j++, sync);
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

		this.hideTarget = function(){
			var i = this.currentIndex,
				target = this.tabs[i] ? this.tabs[i].target : null;

			if (!target || !isDOMElement(target)) return false;
			target.hide();
		};

		this.showTarget = function(){
			var i = this.currentIndex,
				target = this.tabs[i] ? this.tabs[i].target : null;

			if (!target || !isDOMElement(target)) return false;
			target.show();
		};

		this.selectTabAtIndex = function(index){
			var selectedTab = false;
			if (self.currentIndex == index) return false;

			self.hideTarget();
			self.currentIndex = Math.max(Math.min(index, self.tabs.length-1), 0);
			self.currentPosition = self.getPosition(self.currentIndex);

			selectedTab = this.resetTabs();
			self.showTarget();
			
			self.fireEvent("tabselect", {
				tab : selectedTab
			});
			return selectedTab;
		};

		this.selectTabAtPosition = function(p){
			var tab = this.getTabAtPosition(p);
			if (!tab) {
				return false;
			} else if (self.currentPosition == p) {
				return tab;
			} else {
				return this.selectTabAtIndex(tab.index);
			}
		};

		this.getSelectedTab = function(){
			var tab = self.tabs[self.currentIndex];
			return tab ? tab : null;
		};

		this.selectTab = function(tab){
			if (!tab) return false;
			return this.selectTabAtIndex(tab.index);
		};

		this.selectNextTab = function(){
			var p = self.currentPosition,
				tab = self.getTabAtPosition(p+1) ? 
					  self.getTabAtPosition(p+1) : self.getTabAtPosition(p);

			self.selectTab(tab);
		};

		this.selectPreviousTab = function(){
			var p = self.currentPosition,
				tab = self.getTabAtPosition(p-1) ? 
					  self.getTabAtPosition(p-1) : self.getTabAtPosition(p);

			self.selectTab(tab);
		};

		this.removeTabAtPosition = function(p){
			var tab = self.getTabAtPosition(p);
			self._removeTabElement(tab);
		};

		this.removeTab = function(tab){
			if (!tab || this.__removing || this.__removeStarted) return false;

			controller.fireEvent("tabbeforeclose", {
				tab : tab
			}, function(){
				self.__removeStarted = true;
				tab.fadeOut(120, function(){
					self.__removeStarted = false;
					self._removeTabElement(this);
				});
			});
		};

		this._removeTabElement = function(tab){
			if (!tab || this.__removing) return false;

			var index = tab.index,
				to = self.taborder,
				p = self.tabs[index] ? self.tabs[index].pos : null,
				tb = self.tabs;

			if (p == null) return false;

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

			to.splice(p, 1);

			for (var i=index; i<tb.length; i++){
				tb[i].pos--;
				tb[i].index--;
			}

			tb.splice(index, 1);
			
			self.resetTabs(true);

			self.fireEvent("tabclose", {
				tab : tab,
				elements : to
			});

			this.__removing = false;
		};

		this.insertTab = function(p, options){
			var to = self.taborder,
				tb = self.tabs,
				tab = self.getTabAtPosition(p),
				index = tab ? tab.index : 0,
				left = tab && tab.left ? tab.left : 0,
				newtab;

			for (var i=index; i<tb.length; i++){
				tb[i].pos++;
				tb[i].index++;
			}
			tb.splice(index, 0, {});

			self._addTab(index, p, options);

			newtab = tb[index];

			//newtab.__lock("insertTab");
			newtab.left = left;
			//newtab.__unlock("insertTab");

			newtab.opacity = 0;

			for (var i=0; i<to.length; i++){
				if (to[i] >= index){
					to[i]++;
				}
			}
			to.splice(p, 0, index);
			
			self.resetTabs(true, function(){
				newtab.fadeIn(10);
			});

			self.fireEvent("tabinsert", {
				tab : newtab,
				elements : to
			});
		};

		this.swapTabs = function(pos1, pos2){
			var A = self.taborder;
			A[pos1]= A.splice(pos2, 1, A[pos1])[0];

			self.tabs[A[pos1]].pos = pos1;
			self.tabs[A[pos2]].pos = pos2;
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

		this.getTabAtPosition = function(p){
			return self.tabs[ self.taborder[p] ];
		};

		this._addTab = function(i, p, options, left){
			var o = options,
				selected = OptionalBoolean(o.selected, false),
				l = tabs.length;

			if (selected) {
				self.currentIndex = i;
				self.currentPosition = i;
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
			this.tabs[i].pos = p;

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

				controller._oldcursor = window.cursor;
			}, false);

			tab.addEventListener("dragend", function(e){
				if (__dragTabPosition===false) return false;
				
				var curr = controller.getTabAtPosition(__dragTabPosition);

				if (controller._oldcursor) window.cursor = controller._oldcursor;

				curr.slideX(__startX, 200, function(){}, Math.physics.cubicOut);

				__dragTabPosition = false;
				__startX = false;
				__endX = false;

				if (__fireEvent){
					controller.fireEvent("tabmove", {
						tab : curr,
						index : tab.index,
						elements : controller.taborder
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
				dx = e.xrel,

				curr = controller.getTabAtPosition(i),
				next = controller.getTabAtPosition(i+1),
				prev = controller.getTabAtPosition(i-1),

				cx = curr.__left,
				cw = curr.width,

				nx = next ? next.__left : null,
				px = prev ? prev.__left : null;

			window.cursor = "drag";

			if (cx + dx < controller.__left) {
				curr.left = controller.left;
			} else if (cx + cw + dx > controller.__left + controller.width){
				curr.left = controller.width - cw;
			} else {
				curr.left += dx;
			}

			if (next && dx>0 && cx+cw > (nx+next.width/2)){
				i = self.swapWithNext(curr, next, i);
				curr = controller.getTabAtPosition(i);
				next = controller.getTabAtPosition(i+1);
				prev = controller.getTabAtPosition(i-1);

				__startX += prev.width - controller.overlap;
				__endX += prev.width - controller.overlap;
			}

			if (prev && dx<0 && cx < (px+prev.width/2)){
				i = self.swapWithPrev(curr, prev, i);
				curr = controller.getTabAtPosition(i);
				next = controller.getTabAtPosition(i+1);
				prev = controller.getTabAtPosition(i-1);

				__startX -= next.width - controller.overlap;
				__endX -= next.width - controller.overlap;
			}
		}, false);

		this.fireTabSwapEvent = function(tab, from, to){
			this.fireEvent("tabswap", {
				tab : tab,
				from : from,
				to : to
			});
		};

		this.swapWithNext = function(curr, next, i){
			this.currentPosition = __dragTabPosition+1;
			this._slideTab(next, __startX);
			this.swapTabs(i, i+1);
			this.fireTabSwapEvent(curr, i, __dragTabPosition+1);
			__fireEvent = true;
			return ++__dragTabPosition;
		};

		this.swapWithPrev = function(curr, prev, i){
			this.currentPosition = __dragTabPosition-1;
			this._slideTab(prev, __endX - prev.width);
			this.swapTabs(i, i-1);
			this.fireTabSwapEvent(curr, i, __dragTabPosition-1);
			__fireEvent = true;
			return --__dragTabPosition;
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
		DOMElement.draw.box(this, context, params);
	}
});
/* -------------------------- */
/* Native (@) 2013 Stight.com */
/* -------------------------- */

Native.elements.export("UITabController", {
	refresh : function(){
	},

	init : function(){
		var self = this,
			controller = this;

		this.background = OptionalValue(this.options.background, "");
		this.name = OptionalString(this.options.name, "Default");
		this.overlap = OptionalNumber(this.options.overlap, 14);
		this.selection = false;
		this.position = false;

		this.width = OptionalNumber(this.options.width, this.parent.width);
		this.height = OptionalNumber(this.options.height, 32);

		this.tabs = [];
		this.taborder = [];

		this.resetTabs = function(animate, callback){
			var x = 0,
				j = 2,
				nbtabs = self.taborder.length,
				selectedTab = self.tabs[self.selection];

			for (var i=0; i<nbtabs; i++){
				var t = self.taborder[i],
					tab = self.tabs[t];

				tab.position = i;
				tab.selected = false;
				tab.sendToBack();

				if (animate) {
					var _callback = (i==1 ? callback : null);
					tab.slideX(x, 60*j++, _callback);
				} else {
					tab.left = x;
				}
				x += tab.width - this.overlap;
			}

			if (selectedTab) {
				selectedTab.selected = true;
				selectedTab.bringToFront();
			}
		};

		this.selectTab = function(index){
			self.selection = Math.max(Math.min(index, self.tabs.length-1), 0);
			self.resetTabs();
			self.position = self.getPosition(self.selection);

			self.fireEvent("tabselect", {
				index : self.selection,
				position : self.position
			});
		};

		this.selectNextTab = function(){
			var p = self.position, // current selected tab position
				
				// next tab at position p+1
				tab = self.getTab(p+1) ? 
					  self.getTab(p+1) : self.getTab(p);

			self.selectTab(tab.index);
		};

		this.selectPreviousTab = function(){
			var p = self.position, // current selected tab position
			
				// previous tab at position p-1
				tab = self.getTab(p-1) ? 
					  self.getTab(p-1) : self.getTab(p);

			self.selectTab(tab.index);
		};

		this.removeTabAtPosition = function(position){
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
				self._removeTabElement(this);
				//this.remove();
			});
			*/
			self._removeTabElement(tab);
		};

		this._removeTabElement = function(tab){
			var index = tab.index
				to = self.taborder,
				position = self.tabs[index].position,
				tb = self.tabs;

			tab.width = 0;
			tab.height = 0;
			tab.remove();

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
		};

		this.insertTab = function(position, options){
			var to = self.taborder,
				tb = self.tabs,
				tab = self.getTab(position),
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
				if (to[i] >= index){
					to[i]++;
				}
			}
			to.splice(position, 0, index);
			
			self.resetTabs(true, function(){
				/*
				newtab.bounceScale(1, 80, function(){
					//self._removeTabElement(this);
					//this.remove();
				});
				*/
				newtab.show();
			});

			self.fireEvent("tabinsert", {
				index : index,
				positions : to
			});
		};

		this.swapTabs = function(x, y){
			var A = self.taborder;
			A[x]= A.splice(y, 1, A[x])[0];

			self.tabs[A[x]].position = x;
			self.tabs[A[y]].position = y;
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
				height : this.height - 8,
				selected : selected,

				label : OptionalString(o.label, "New Tab"),
				background : OptionalValue(o.background, "#262722"),
				color : OptionalValue(o.color, "#abacaa"),
				closable : OptionalBoolean(o.closable, true),
				preventmove : OptionalBoolean(o.preventmove, false)
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
			tab.addEventListener("dragstart", function(e){
				__fireEvent = false;
				__startX = tab.left;
				__endX = tab.left + tab.width;
				__dragTabPosition = controller.getPosition(tab.index);
			}, false);

			tab.addEventListener("dragend", function(e){
				if (__dragTabPosition===false) { return false; }
				
				var curr = controller.getTab(__dragTabPosition);

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

			tab.closeButton.addEventListener("mouseup", function(){
				/*
				this.parent.g = {
					x : 0,
					y : this.parent.height/2
				};
				this.parent.bounceScale(0, 120, function(){
					this.parent._removeTab(this);
				});
				*/
				controller._removeTabElement(tab);
			}, false);

		};

		this._root.addEventListener("dragover", function(e){
			if (__dragTabPosition===false) { return false; }

			var i = __dragTabPosition,
				curr = controller.getTab(i),
				next = controller.getTab(i+1),
				prev = controller.getTab(i-1),

				cx = curr._absx,
				cw = curr.width,

				nx = next ? next._absx : null,
				px = prev ? prev._absx : null;

			if (cx + e.xrel < controller._absx) {
				curr.left = controller.left;
			} else if (cx + cw + e.xrel > controller._absx + controller.width){
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
				curr = controller.getTab(i);
				next = controller.getTab(i+1);
				prev = controller.getTab(i-1);

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
				curr = controller.getTab(i);
				next = controller.getTab(i+1);
				prev = controller.getTab(i-1);

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
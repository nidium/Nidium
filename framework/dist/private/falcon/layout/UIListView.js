/* ------------------------+------------- */
/* Native Framework 2.0    | Falcon Build */
/* ------------------------+------------- */
/* (c) 2013 nidium.com - Vincent Fontaine */
/* -------------------------------------- */

Native.elements.export("UIListView", {
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
			tabs = o.elements ? o.elements : [];

		this.selection = 0;
		this.tabs = [];

		this.setProperties({
			fontSize  		: OptionalNumber(o.fontSize, 11),
			fontType  		: OptionalString(o.fontType, "arial"),

			paddingLeft		: OptionalNumber(o.paddingLeft, 10),
			paddingRight	: OptionalNumber(o.paddingLeft, 10),

			maxHeight 		: OptionalNumber(o.maxHeight, null),
			radius 			: OptionalNumber(o.radius, 2),
			background 		: OptionalValue(o.background, "#2277E0"),
			color 			: OptionalValue(o.color, "#ffffff"),

			name : OptionalString(o.name, "Default")
		});

		this.getComputedHeight = function(){
			var	l = this.tabs.length,
				h = l*this.height;

			return this.maxHeight || h;
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
				background = OptionalValue(o.background, "rgba(255, 255, 255, 1)"),
				className = OptionalString(o.class, ""),
				value = OptionalValue(o.value, ""),
				color = OptionalValue(o.color, "#555555"),
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

			this.tabs[i].addEventListener("mousedown", function(e){
				if (this.disabled) {
				} else {
					self.selectIndex(this.index);
					if (self.parent && self.parent.closeSelector) {
						self.parent.closeSelector();021
					}
				}
				e.stopPropagation();
			}, false);
		};

		this.selector = this.add("UIView", {
			left : 0,
			top : 0,
			width : this.width,
			height : this.getComputedHeight(),
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
			}
			return this;
		};

		this.reset();
	},

	draw : function(context){}
});
/* ------------------------+------------- */
/* Native Framework 2.0    | Falcon Build */
/* ------------------------+------------- */
/* (c) 2013 Stight.com - Vincent Fontaine */
/* -------------------------------------- */

Native.elements.export("UIOption", {
	init : function(){
		var self = this,
			o = this.options,
			controller = this.parent.parent;

		this.setProperties({
			label			: OptionalString(o.label, "Default"),
			fontSize  		: OptionalNumber(o.fontSize, 11),
			fontType  		: OptionalString(o.fontType, "arial"),

			paddingLeft		: OptionalNumber(o.paddingLeft, 8),
			paddingRight	: OptionalNumber(o.paddingLeft, 8),

			height 			: OptionalNumber(o.height, 22),
			radius 			: OptionalNumber(o.radius, 0),
			background 		: OptionalValue(o.background, "rgba(0, 0, 0, 0.4)"),
			color 			: OptionalValue(o.color, "#abacaa"),

			value 			: OptionalValue(o.value, ""),
			disabled		: OptionalBoolean(o.disabled, false),

			cursor			: OptionalCursor(o.cursor, "pointer")
		});

		this.addEventListener("mousedown", function(e){
			if (this.disabled) {
			} else {
				controller.selectIndex(this.index);
				controller.closeSelector();
			}
			e.stopPropagation();
		}, false);

		this.addEventListener("contextmenu", function(e){
			e.preventDefault();
		}, false);

		DOMElement.listeners.addHovers(this);

		this.updateElement = function(){
			if (this.disabled) this.color = "#aaaaaa";
			this.width = this.parent.width;
			this.height = this.parent.parent.height;
		};

		this.updateElement();
	},

	draw : function(context){
		var	params = this.getDrawingBounds(),
			color = this.selected ? this.parent.parent.selectedColor
								  : this.color;

		DOMElement.draw.box(this, context, params);

		var gradient = context.createLinearGradient(
			params.x, params.y, 
			params.x, params.y+params.h
		);

		if (this.selected){
			gradient = this.parent.parent.selectedBackground;
		} else {
			if (this.hover){
				gradient.addColorStop(0.00, 'rgba(128, 128, 128, 0.25)');
				gradient.addColorStop(1.00, 'rgba(128, 128, 128, 0.15)');
			} if (this.disabled) {
				gradient.addColorStop(0.00, 'rgba(128, 128, 128, 0.35)');
				gradient.addColorStop(1.00, 'rgba(128, 128, 128, 0.35)');
			} else {
				gradient.addColorStop(0.00, 'rgba(128, 128, 128, 0.10)');
				gradient.addColorStop(0.10, 'rgba(128, 128, 128, 0.05)');
			}
		}

		DOMElement.draw.box(this, context, params, gradient);
		DOMElement.draw.label(this, context, params, color);
	}
});
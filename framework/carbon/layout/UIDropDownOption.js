/* -------------------------- */
/* Native (@) 2012 Stight.com */
/* -------------------------- */

Native.elements.export("UIDropDownOption", {
	init : function(){
		var self = this,
			controller = this.parent.parent;

		this.w = this.parent.w;
		this.h = this.parent.parent.h;

		this.addEventListener("mousedown", function(e){
			controller.selectTab(this.tabnum);
			e.stopPropagation();
		}, false);

		this.addEventListener("mouseover", function(e){
			this.hover = true;
		});

		this.addEventListener("mouseout", function(e){
			this.hover = false;
		});

		this.clip = {
			x : this.parent.__x,
			y : this.parent.__y,
			w : this.parent.__w,
			h : this.parent.__h
		};

	},

	draw : function(){
		var context = this.layer.context,
			params = {
				x : this._x,
				y : this._y,
				w : this.w,
				h : this.h
			},

			label = this.label,
			textWidth = Math.round(context.measureText(label)),
			textHeight = 10,
			w = params.w,
			h = params.h,
			textOffsetX = 7,
			textOffsetY = (h-textHeight)/2 + 9,
			textColor = this.color,
			textShadow = '#000000';


		this.clip = {
			x : this.parent.__x,
			y : this.parent.__y,
			w : this.parent.__w,
			h : this.parent.__h
		};
		
		context.roundbox(params.x, params.y, w, h, 0, this.background, false);

		var gradient = context.createLinearGradient(
			params.x, params.y, 
			params.x, params.y+params.h
		);

		if (this.selected){
			//textOffsetY++;
			textColor = this.parent.parent.selectedColor;
			textShadow = "rgba(0, 0, 0, 0.10)";
			/*
			gradient.addColorStop(0.00, 'rgba(40, 60, 255, 0.90)');
			gradient.addColorStop(0.10, 'rgba(40, 60, 255, 0.90)');
			gradient.addColorStop(0.90, 'rgba(40, 60, 255, 0.90)');
			*/
			gradient = this.parent.parent.selectedBackground;

		} else {

			textColor = this.color;
			textShadow = "rgba(0, 0, 0, 0.35)";

			if (this.hover){
				gradient.addColorStop(0.00, 'rgba(255, 255, 255, 0.25)');
				gradient.addColorStop(1.00, 'rgba(255, 255, 255, 0.15)');
			} else {
				gradient.addColorStop(0.00, 'rgba(255, 255, 255, 0.10)');
				gradient.addColorStop(0.10, 'rgba(255, 255, 255, 0.05)');
			}

		}

		context.roundbox(params.x, params.y, w, h, 0, gradient, false);

		context.setFontSize(11);
		//context.setColor(textShadow);
		//context.fillText(label, params.x+textOffsetX+1, params.y+textOffsetY+1);

		context.setColor(textColor);
		context.fillText(label, params.x+textOffsetX, params.y+textOffsetY);

	}
});
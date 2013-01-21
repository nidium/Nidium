/* -------------------------- */
/* Native (@) 2012 Stight.com */
/* -------------------------- */

Native.elements.export("UISliderKnob", {
	init : function(){
		this.w = this.options.w || 10;
		this.h = this.options.h || 10;

		this.addEventListener("mousedown", function(e){
			this.selected = true;
		});

		this.addEventListener("mouseup", function(e){
			this.selected = false;
		});

		this.addEventListener("dragend", function(e){
			this.selected = false;
		});

		this.addEventListener("mouseover", function(e){
			this.hover = true;
		});

		this.addEventListener("mouseout", function(e){
			this.hover = false;
		});

	},

	draw : function(){
		var context = this.layer.context,
			params = {
				x : this._x,
				y : this._y,
				w : this.w,
				h : this.h
			},

			radius = this.w/2;

		if (this.background!='') {
			var gradient = context.createRadialGradient(
				params.x+radius, params.y+radius, 
				radius, params.x+radius-1, params.y+radius-2, radius/8
			);

			if (this.hover){
				gradient.addColorStop(0.00, 'rgba(0, 0, 0, 0.1)');
				gradient.addColorStop(0.20, 'rgba(255, 255, 255, 0.1)');
				gradient.addColorStop(1.00, 'rgba(255, 255, 255, 0.9)');
			} else {
				gradient.addColorStop(0.00, 'rgba(0, 0, 0, 0.1)');
				gradient.addColorStop(0.40, 'rgba(255, 255, 255, 0.1)');
				gradient.addColorStop(1.00, 'rgba(255, 255, 255, 0.4)');
			}

	        context.lineWidth = 1;
			context.setShadow(-1, 1, 3, "rgba(0, 0, 0, 0.70)");

	        context.setColor(this.background);
	        context.beginPath();
	        context.arc(
	        	params.x+radius, params.y+params.h*0.5, 
	        	radius, 0, 6.2831852, false
	        );
	        context.fill();

			context.setShadow(0, 0, 0);

	        context.setColor(gradient);
	        context.beginPath();
	        context.arc(
	        	params.x+radius, params.y+params.h*0.5, 
	        	radius, 0, 6.2831852, false
	        );
	        context.fill();
		}

	}
});

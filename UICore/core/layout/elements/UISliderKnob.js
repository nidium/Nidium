/* -------------------------- */
/* Native (@) 2012 Stight.com */
/* -------------------------- */

UIElement.extend("UISliderKnob", {
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
		var params = {
				x : this._x,
				y : this._y,
				w : this.w,
				h : this.h
			},

			radius = this.w/2;

		if (this.background!='') {
			var gdBackground = canvas.createRadialGradient(params.x+radius, params.y+radius, radius, params.x+radius-1, params.y+radius-2, radius/8);

			if (this.hover){
				gdBackground.addColorStop(0.00, 'rgba(0, 0, 0, 0.1)');
				gdBackground.addColorStop(0.40, 'rgba(255, 255, 255, 0.1)');
				gdBackground.addColorStop(1.00, 'rgba(255, 255, 255, 0.4)');
			} else {
				gdBackground.addColorStop(0.00, 'rgba(0, 0, 0, 0.1)');
				gdBackground.addColorStop(0.20, 'rgba(255, 255, 255, 0.1)');
				gdBackground.addColorStop(1.00, 'rgba(255, 255, 255, 0.9)');
			}

	        canvas.lineWidth = 1;
			canvas.setShadow(0, 0, 3, "rgba(0, 0, 0, 0.50)");

	        canvas.setColor(this.background);
	        canvas.beginPath();
	        canvas.arc(params.x+radius, params.y+params.h*0.5, radius, 0, 6.2831852, false);
	        canvas.fill();

			canvas.setShadow(0, 0, 0);

	        canvas.setColor(gdBackground);
	        canvas.beginPath();
	        canvas.arc(params.x+radius, params.y+params.h*0.5, radius, 0, 6.2831852, false);
	        canvas.fill();
		}

	}
});

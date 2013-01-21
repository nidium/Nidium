/* -------------------------- */
/* Native (@) 2012 Stight.com */
/* -------------------------- */

Native.elements.export("UIButtonDown", {
	init : function(){
		this.w = OptionalNumber(this.options.w, 10);
		this.h = OptionalNumber(this.options.h, 10);

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
				radius, params.x+radius, params.y+radius, radius/4
			);

			if (this.hover){
				gradient.addColorStop(0.00, 'rgba(0, 0, 0, 0.01)');
				gradient.addColorStop(1.00, 'rgba(0, 0, 0, 0.1)');
			} else {
				gradient.addColorStop(0.00, 'rgba(255, 255, 255, 0.01)');
				gradient.addColorStop(1.00, 'rgba(255, 255, 255, 0.1)');
			}

	        context.beginPath();
	        context.arc(
	        	params.x+radius, params.y+params.h*0.5, 
	        	radius, 0, 6.2831852, false
	        );
	        context.setColor(this.background);
	        context.fill();
	        context.lineWidth = 1;

	        context.beginPath();
	        context.arc(
	        	params.x+radius, params.y+params.h*0.5, 
	        	radius, 0, 6.2831852, false
	        );
	        context.setColor(gradient);
	        context.fill();
	        context.lineWidth = 1;

	        if (this.selected){
		        context.beginPath();
		        context.arc(
		        	params.x+radius, params.y+params.h*0.5, 
		        	radius, 0, 6.2831852, false
		        );
		        context.setColor(this.background);
		        context.fill();
		        context.lineWidth = 1;
	        }
		}

        var m = radius/1.8,
        	x1 = params.x+m - 0.25,
        	y1 = params.y+m + 1,

        	x2 = params.x+params.w - m + 0.4,
        	y2 = y1,

        	x3 = x1+(x2-x1)/2,
        	y3 = params.y+params.h - m;

		context.strokeStyle = this.hover ? "#ffffff" : this.color;
        context.lineWidth = 0.5;

		context.lineCap = "round"; // default : butt
		context.lineJoin = "round"; // default : miter

		context.beginPath();
		context.moveTo(x1, y1);
		context.lineTo(x2, y2);
		context.lineTo(x3, y3);
		context.lineTo(x1, y1);
		context.stroke();
        context.setColor(this.hover ? "#ffffff" : this.color);
		context.fill();
	}
});

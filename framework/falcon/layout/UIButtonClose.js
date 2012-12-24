/* -------------------------- */
/* Native (@) 2013 Stight.com */
/* -------------------------- */

Native.elements.export("UIButtonClose", {
	init : function(){
		this.width = OptionalNumber(this.options.width, 10);
		this.height = OptionalNumber(this.options.height, 10);

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

	draw : function(context){
		var	params = this.getDrawingBounds(),
			radius = this.width/2;

		if (this.background!='') {
			var gradient = context.createRadialGradient(
				params.x+radius,
				params.y+radius,
				radius,
				params.x+radius,
				params.y+radius,
				radius/4
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

        var m = radius/1.6,
        	x1 = params.x+m,
        	y1 = params.y+m,
        	x2 = params.x+params.w - m,
        	y2 = params.y+params.h - m;

		context.strokeStyle = this.hover ? "#ffffff" : this.color;
        context.lineWidth = 2;

		context.beginPath();
		context.moveTo(x1, y1);
		context.lineTo(x2, y2);
		context.stroke();

		context.beginPath();
		context.moveTo(x1, y2);
		context.lineTo(x2, y1);
		context.stroke();

	}
});

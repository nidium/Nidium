/* -------------------------- */
/* Native (@) 2012 Stight.com */
/* -------------------------- */

UIElement.extend("UIButtonDown", {
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

			textColor = "rgba(255, 255, 255, 0.8)",
			textShadow = "rgba(0, 0, 0, 0.15)",
			radius = this.w/2;

		if (this.background!='') {
			var gdBackground = canvas.createRadialGradient(params.x+radius, params.y+radius, radius, params.x+radius, params.y+radius, radius/4);

			if (this.hover){
				gdBackground.addColorStop(0.00, 'rgba(0, 0, 0, 0.01)');
				gdBackground.addColorStop(1.00, 'rgba(0, 0, 0, 0.1)');
			} else {
				gdBackground.addColorStop(0.00, 'rgba(255, 255, 255, 0.01)');
				gdBackground.addColorStop(1.00, 'rgba(255, 255, 255, 0.1)');
			}

	        canvas.beginPath();
	        canvas.arc(params.x+radius, params.y+params.h*0.5, radius, 0, 6.2831852, false);
	        canvas.fillStyle = this.background;
	        canvas.fill();
	        canvas.lineWidth = 1;

	        canvas.beginPath();
	        canvas.arc(params.x+radius, params.y+params.h*0.5, radius, 0, 6.2831852, false);
	        canvas.fillStyle = gdBackground;
	        canvas.fill();
	        canvas.lineWidth = 1;

	        if (this.selected){
		        canvas.beginPath();
		        canvas.arc(params.x+radius, params.y+params.h*0.5, radius, 0, 6.2831852, false);
		        canvas.fillStyle = this.background;
		        canvas.fill();
		        canvas.lineWidth = 1;
	        }
		}

        var m = radius/1.8,
        	x1 = params.x+m - 0.25,
        	y1 = params.y+m + 1,

        	x2 = params.x+params.w - m + 0.4,
        	y2 = y1,

        	x3 = x1+(x2-x1)/2,
        	y3 = params.y+params.h - m;

        /*
        canvas.lineWidth = 1;
		canvas.strokeStyle = 'rgba(0, 0, 0, 0.7)';
		canvas.beginPath();
		canvas.moveTo(x1+0.5, y2+0.5);
		canvas.lineTo(x2+1, y1+1);
		canvas.stroke();

        canvas.lineWidth = 1;
		canvas.strokeStyle = 'rgba(0, 0, 0, 0.7)';
		canvas.beginPath();
		canvas.moveTo(x1-1, y1+1);
		canvas.lineTo(x2-0.5, y2+0.5);
		canvas.stroke();
		*/

		canvas.strokeStyle = this.hover ? "#ffffff" : this.color;
        canvas.lineWidth = 0.5;

		canvas.lineCap = "round"; // default : butt
		canvas.lineJoin = "round"; // default : miter

		canvas.beginPath();
		canvas.moveTo(x1, y1);
		canvas.lineTo(x2, y2);
		canvas.lineTo(x3, y3);
		canvas.lineTo(x1, y1);
		canvas.stroke();
        canvas.fillStyle = this.hover ? "#ffffff" : this.color;
		canvas.fill();
	}
});

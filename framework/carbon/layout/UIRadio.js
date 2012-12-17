/* -------------------------- */
/* Native (@) 2012 Stight.com */
/* -------------------------- */

Native.elements.export("UIRadio", {
	init : function(){
		this.w = 16;
		this.h = 17;
		this.flags._canReceiveFocus = true;

		this.addEventListener("mousedown", function(e){

			Native.layout.find("name", "choice").each(function(){
				this.selected = false;
			});

			this.selected = true;
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

			textColor = "rgba(255, 255, 255, 0.8)",
			textShadow = "rgba(0, 0, 0, 0.15)",
			radius = this.w/2;

		context.setFontSize(this.fontSize);

		var gradient = context.createRadialGradient(
			params.x+radius, 
			params.y+radius, 
			radius, 
			params.x+radius*0.7, 
			params.y+radius*0.7, 
			radius>>1
		);

		gradient.addColorStop(0.00,'#ccccff');
		gradient.addColorStop(1.00,'#ffffff');

        context.beginPath();
        
        context.arc(
        	params.x+radius, 
        	params.y+params.h*0.5, 
        	radius, 0, 6.2831852, false
        );

        context.setColor(gradient);
        context.fill();
        context.lineWidth = 1;
        context.strokeStyle = "rgba(140, 140, 140, 0.7)";
        context.stroke();

        if (this.selected){
	        var r = 4;
	        context.beginPath();

	        context.arc(
	        	params.x+radius, 
	        	params.y+params.h*0.5, 
	        	radius-r, 0, 6.2831852, false
	        );

	        context.setColor("rgba(60, 80, 200, 0.5)");
	        context.fill();
	        context.lineWidth = 1;
    	    context.strokeStyle = "rgba(0, 0, 180, 0.1)";
	        context.stroke();
        }


		context.textAlign = "center";
		context.textBaseline = 'center';

		context.setColor(textShadow);
		
		context.fillText(
			this.label, 
			params.x+params.w+8, 
			params.y+5+params.h*0.5
		);

		context.setColor(textColor);
		context.fillText(
			this.label, 
			params.x+params.w+8, 
			params.y+5+params.h*0.5
		);

	}
});

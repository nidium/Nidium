/* -------------------------- */
/* Native (@) 2012 Stight.com */
/* -------------------------- */

/*
canvas.fontName = "arial";
canvas.fontSize = 13.5;
canvas.fontWeight = "bold";
canvas.fontStyle = "italic";
*/

Native.elements.export("UIRadio", {
	init : function(){
		this.w = 16;
		this.h = 17;
		this.flags._canReceiveFocus = true;

		this.addEventListener("mousedown", function(e){

			Native.document.find("name", "choice").each(function(){
				this.selected = false;
			});

			this.selected = true;
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

		var gBackground = canvas.createRadialGradient(params.x+radius, params.y+radius, radius, params.x+radius*0.7, params.y+radius*0.7, radius>>1);
		gBackground.addColorStop(0.00,'#ccccff');
		gBackground.addColorStop(1.00,'#ffffff');

        canvas.beginPath();
        canvas.arc(params.x+radius, params.y+params.h*0.5, radius, 0, 6.2831852, false);
        canvas.setColor(gBackground);
        canvas.fill();
        canvas.lineWidth = 1;
        canvas.strokeStyle = "rgba(140, 140, 140, 0.7)";
        canvas.stroke();

        if (this.selected){
	        var r = 4;
	        canvas.beginPath();
	        canvas.arc(params.x+radius, params.y+params.h*0.5, radius-r, 0, 6.2831852, false);
	        canvas.setColor("rgba(60, 80, 200, 0.5)");
	        canvas.fill();
	        canvas.lineWidth = 1;
    	    canvas.strokeStyle = "rgba(0, 0, 180, 0.1)";
	        canvas.stroke();
        }


		canvas.textAlign = "center";
		canvas.textBaseline = 'center';

		canvas.setColor(textShadow);
		canvas.fillText(this.label, params.x+params.w+8, params.y+5+params.h*0.5);

		canvas.setColor(textColor);
		canvas.fillText(this.label, params.x+params.w+8, params.y+5+params.h*0.5);

	}
});

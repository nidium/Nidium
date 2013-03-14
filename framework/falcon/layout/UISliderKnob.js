/* ------------------------+------------- */
/* Native Framework 2.0    | Falcon Build */
/* ------------------------+------------- */
/* (c) 2013 Stight.com - Vincent Fontaine */
/* -------------------------------------- */

Native.elements.export("UISliderKnob", {
	init : function(){
		var o = this.options;
		this.width = OptionalNumber(o.width, 10);
		this.height = OptionalNumber(o.height, 10);
		this.cursor = OptionalCursor(o.cursor, "pointer");

		DOMElement.listeners.addDefault(this);
	},

	draw : function(context){
		var	params = this.getDrawingBounds(),
			radius = this.width/2;

		if (this.background!=''){
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
			context.setShadow(-1, 2, 3, "rgba(0, 0, 0, 0.70)");

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

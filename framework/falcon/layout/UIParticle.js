/* ------------------------+------------- */
/* Native Framework 2.0    | Falcon Build */
/* ------------------------+------------- */
/* (c) 2013 Stight.com - Vincent Fontaine */
/* -------------------------------------- */

Native.elements.export("UIParticle", {
	init : function(){
		var o = this.options;
		this.radius = OptionalNumber(o.radius, 10);
		this.background = OptionalValue(o.background, "#ffffff");

		this.shadowBlur = OptionalNumber(o.shadowBlur, 30);
		this.shadowColor = OptionalValue(o.shadowColor, "rgba(255, 255, 255, 1");

		this.width = 2*this.radius;
		this.height = 2*this.radius;
	},

	draw : function(context){
		var	params = this.getDrawingBounds();
		
		context.setShadow(
			this.shadowOffsetX,
			this.shadowOffsetY,
			this.shadowBlur,
			this.shadowColor
		);

		context.setShadow(
			this.shadowOffsetX,
			this.shadowOffsetY,
			this.shadowBlur,
			this.shadowColor
		);


		context.beginPath();
		context.arc(
			params.x+this.radius, params.y+params.h*0.5, 
			this.radius/2, 0, 6.2831852, false
		);
		context.setColor(this.background);
		context.fill();

		context.beginPath();
		context.arc(
			params.x+this.radius, params.y+params.h*0.5, 
			this.radius/4, 0, 6.2831852, false
		);
		context.setColor("#ffffff");
		context.fill();

		
		var gradient = context.createRadialGradient(
			params.x+this.radius,
			params.y+this.radius, 
			this.radius,
			params.x+this.radius,
			params.y+this.radius,
			this.radius/4
		);

		gradient.addColorStop(0.00, 'rgba(255, 255, 255, 0)');
		gradient.addColorStop(0.70, 'rgba(255, 255, 255, 0.1)');
		gradient.addColorStop(1.00, 'rgba(255, 255, 255, 0.6)');

		context.beginPath();
		context.arc(
			params.x+this.radius, params.y+params.h*0.5, 
			this.radius, 0, 6.2831852, false
		);
		context.setColor(gradient);
		context.fill();

		context.setShadow(0, 0, 0);

		context.globalAlpha = 0.9;
		context.beginPath();
		context.setColor(this.background);
		context.strokeStyle = "rgba(255, 255, 255, 0.02";
		context.arc(
			params.x+this.radius, params.y+params.h*0.5, 
			this.radius*3, 0, 6.2831852, false
		);
		context.stroke();

		context.globalAlpha = 0.1;
		context.beginPath();
		context.setColor(this.background);
		context.strokeStyle = "rgba(255, 255, 255, 0.05";
		context.arc(
			params.x+this.radius, params.y+params.h*0.5, 
			this.radius*3, 0, 6.2831852, false
		);
		context.fill();

	}
});

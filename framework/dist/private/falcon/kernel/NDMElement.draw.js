/* ------------------------+------------- */
/* Native Framework 2.0    | Falcon Build */
/* ------------------------+------------- */
/* (c) 2013 nidium.com - Vincent Fontaine */
/* -------------------------------------- */

"use strict";

/* -------------------------------------------------------------------------- */

NDMElement.draw = {
	circleBackground : function(element, context, params, radius){
		var gradient = context.createRadialGradient(
			params.x+radius,
			params.y+radius, 
			radius,
			params.x+radius,
			params.y+radius,
			radius/4
		);

		if (element.hover){
			gradient.addColorStop(0.00, 'rgba(0, 0, 0, 0.01)');
			gradient.addColorStop(1.00, 'rgba(0, 0, 0, 0.1)');
		} else {
			gradient.addColorStop(0.00, 'rgba(255, 255, 255, 0.01)');
			gradient.addColorStop(1.00, 'rgba(255, 255, 255, 0.1)');
		}

		context.beginPath();
		context.arc(
			params.x+radius, params.y+params.h*0.5, 
			radius, 0, 6.283185307179586, false
		);
		context.setColor(element.background);
		context.fill();
		context.lineWidth = 1;

		context.beginPath();
		context.arc(
			params.x+radius, params.y+params.h*0.5, 
			radius, 0, 6.283185307179586, false
		);
		context.setColor(gradient);
		context.fill();
		context.lineWidth = 1;

		if (element.selected){
			context.beginPath();
			context.arc(
				params.x+radius, params.y+params.h*0.5, 
				radius, 0, 6.283185307179586, false
			);
			context.setColor(element.background);
			context.fill();
			context.lineWidth = 1;
		}
	},

	label : function(element, context, params, color, shadowColor){
		var textOffsetX = element.paddingLeft,
			textOffsetY = (params.h-element.lineHeight)/2 + 4 + element.lineHeight/2;

		var tx = params.x+textOffsetX,
			ty = params.y+textOffsetY;

		if (element.textAlign == "right") {
			tx = params.x + params.w - element._textWidth - element.paddingRight;
		}

		context.fontSize = element.fontSize;
		context.fontFamily = element.fontFamily;

		context.setText(
			element.label,
			params.x+textOffsetX+element.textOffsetX,
			params.y+textOffsetY+element.textOffsetY,
			color ? color : element.color,
			element.textShadowOffsetX,
			element.textShadowOffsetY,
			element.textShadowBlur,
			shadowColor ? shadowColor : element.textShadowColor
		);

	},

	outline : function(element, color){
		var params = element.getDrawingBounds(),
			context = element.layer.context,
			outlineColor = color ? color : element.outlineColor;

		NDMElement.draw.outlineBox(element, context, params, outlineColor);
		NDMElement.draw.outlineBox(element, context, params, outlineColor);
		NDMElement.draw.outlineBox(element, context, params, "rgba(255, 255, 255, 0.8)");
		NDMElement.draw.outlineBox(element, context, params, "rgba(255, 255, 255, 1)");
	},

	outlineBox : function(element, context, params, borderColor){
		context.setShadow(
			0, 0, 4,
			borderColor ? borderColor : "rgba(0, 0, 255, 0.4)"
		);

		context.roundbox(
			params.x-2, params.y-2, 
			params.w+4, params.h+4, 
			element.radius+2,
			"rgba(255, 255, 255, 0.6)",
			null,
			element.borderWidth
		);
		context.setShadow(0, 0, 0);
	},

	box : function(element, context, params, backgroundColor, borderColor){
		context.roundbox(
			params.x, params.y, 
			params.w, params.h, 
			element.radius,
			backgroundColor ? backgroundColor : element.background,
			borderColor ? borderColor : element.borderColor,
			element.borderWidth
		);
	},

	enableShadow : function(element){
		if (__ENABLE_CONTROL_SHADOWS__) {
			element.layer.context.setShadow(
				element.shadowOffsetX,
				element.shadowOffsetY,
				element.shadowBlur,
				element.shadowColor
			);
		}
	},

	disableShadow : function(element){
		element.layer.context.setShadow(0, 0, 0);
	},

	glassLayer : function(element, context, params){
		var gradient = this.getGlassGradient(element, context, params);
		this.box(element, context, params, gradient);
	},

	getGlassGradient : function(element, context, params){
		var gradient = context.createLinearGradient(
			params.x, params.y,
			params.x, params.y+params.h
		);

		if (element.selected){
			gradient.addColorStop(0.00, 'rgba(0, 0, 0, 0.8)');
			gradient.addColorStop(0.25, 'rgba(0, 0, 0, 0.6)');
			gradient.addColorStop(1.00, 'rgba(0, 0, 0, 0.6)');
		} else {
			if (element.hover){
				gradient.addColorStop(0.00, 'rgba(255, 255, 255, 0.7)');
				gradient.addColorStop(0.10, 'rgba(255, 255, 255, 0.3)');
				gradient.addColorStop(0.50, 'rgba(255, 255, 255, 0.1)');
				gradient.addColorStop(0.80, 'rgba(0, 0, 0, 0.1)');
				gradient.addColorStop(1.00, 'rgba(0, 0, 0, 0.3)');
			} else {
				gradient.addColorStop(0.00, 'rgba(255, 255, 255, 0.50)');
				gradient.addColorStop(0.10, 'rgba(255, 255, 255, 0.20)');
				gradient.addColorStop(0.40, 'rgba(255, 255, 255, 0.00)');
				gradient.addColorStop(0.50, 'rgba(0, 0, 0, 0.0)');
				gradient.addColorStop(0.90, 'rgba(0, 0, 0, 0.1)');
				gradient.addColorStop(1.00, 'rgba(0, 0, 0, 0.3)');
			}
		}
		return gradient;
	},

	getSmoothGradient : function(element, context, params){
		var gradient = context.createLinearGradient(
			params.x, params.y,
			params.x, params.y+params.h
		);

		gradient.addColorStop(0.00, 'rgba(255, 255, 255, 0.08)');
		gradient.addColorStop(1.00, 'rgba(0, 0, 0, 0.25)');

		return gradient;
	},

	getSoftGradient : function(element, context, params){
		var gradient = context.createLinearGradient(
			params.x, params.y, 
			params.x, params.y+params.h
		);

		if (element.selected){
			gradient.addColorStop(0.00, 'rgba(255, 255, 255, 0.20)');
			gradient.addColorStop(0.10, 'rgba(255, 255, 255, 0.05)');
			gradient.addColorStop(0.90, 'rgba(255, 255, 255, 0.00)');

		} else {
			if (element.hover){
				gradient.addColorStop(0.00, 'rgba(255, 255, 255, 0.30)');
				gradient.addColorStop(0.25, 'rgba(255, 255, 255, 0.18)');
			} else {
				gradient.addColorStop(0.00, 'rgba(255, 255, 255, 0.25)');
				gradient.addColorStop(0.10, 'rgba(255, 255, 255, 0.15)');
			}
		}
		return gradient;
	},

	getCleanGradient : function(element, context, params){
		var gradient = context.createLinearGradient(
			params.x, params.y, 
			params.x, params.y+params.h
		);

		gradient.addColorStop(0.00, 'rgba(255, 255, 255, 0.20)');
		gradient.addColorStop(0.10, 'rgba(255, 255, 255, 0.05)');
		gradient.addColorStop(0.90, 'rgba(255, 255, 255, 0.00)');
		return gradient;
	},

	getInnerTextWidth : function(element){
		var w = Native.getTextWidth(
			element._label,
			element._fontSize,
			element._fontFamily
		);
		return element._paddingLeft + Math.round(w) + element._paddingRight;
	}
};

/* -------------------------------------------------------------------------- */
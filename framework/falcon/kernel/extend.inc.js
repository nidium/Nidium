/* ------------------------+------------- */
/* Native Framework 2.0    | Falcon Build */
/* ------------------------+------------- */
/* (c) 2013 Stight.com - Vincent Fontaine */
/* -------------------------------------- */

"use strict";

/* -------------------------------------------------------------------------- */

DOMElement.listeners = {
	addDefault : function(element){
		this.addSelectors(element);
		this.addHovers(element);
	},

	addSelectors : function(element){
		element.addEventListener("mousedown", function(e){
			this.selected = true;
		});

		element.addEventListener("mouseup", function(e){
			this.selected = false;
		});

		element.addEventListener("dragend", function(e){
			this.selected = false;
		});
	},

	addHovers : function(element){
		element.addEventListener("mouseover", function(e){
			this.hover = true;
		});

		element.addEventListener("mouseout", function(e){
			this.hover = false;
		});
	}
};

DOMElement.draw = {
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
			radius, 0, 6.2831852, false
		);
		context.setColor(element.background);
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

		if (element.selected){
			context.beginPath();
			context.arc(
				params.x+radius, params.y+params.h*0.5, 
				radius, 0, 6.2831852, false
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

		context.setFontSize(element.fontSize);
		context.setFontType(element.fontType);

		context.setText(
			element.label,
			params.x+textOffsetX+params.textOffsetX,
			params.y+textOffsetY+params.textOffsetY,
			color ? color : element.color,
			shadowColor ? shadowColor : element.textShadowColor
		);

	},

	box : function(element, context, params, backgroundColor){
		context.roundbox(
			params.x, params.y, 
			params.w, params.h, 
			element.radius,
			backgroundColor ? backgroundColor : element.background,
			false
		);
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
				gradient.addColorStop(0.50, 'rgba(255, 255, 255, 0.3)');
				gradient.addColorStop(0.50, 'rgba(255, 255, 255, 0.1)');
				gradient.addColorStop(0.80, 'rgba(0, 0, 0, 0.1)');
				gradient.addColorStop(1.00, 'rgba(0, 0, 0, 0.3)');
			} else {
				gradient.addColorStop(0.00, 'rgba(255, 255, 255, 0.4)');
				gradient.addColorStop(0.50, 'rgba(255, 255, 255, 0.1)');
				gradient.addColorStop(0.50, 'rgba(255, 255, 255, 0.0)');
				gradient.addColorStop(0.50, 'rgba(0, 0, 0, 0.0)');
				gradient.addColorStop(0.80, 'rgba(0, 0, 0, 0.1)');
				gradient.addColorStop(1.00, 'rgba(0, 0, 0, 0.3)');
			}
		}
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
			element._fontType
		);
		return element._paddingLeft + Math.round(w) + element._paddingRight;
	}
};

/* -------------------------------------------------------------------------- */

DOMElement.defineNativeProperty = function(descriptor){
	var element = descriptor.element, // target element
		property = descriptor.property, // property
		value = OptionalValue(descriptor.value, null), // default value
		setter = OptionalCallback(descriptor.setter, null),
		getter = OptionalCallback(descriptor.getter, null),
		writable = OptionalBoolean(descriptor.writable, true),
		enumerable = OptionalBoolean(descriptor.enumerable, true),
		configurable = OptionalBoolean(descriptor.configurable, false);

	if (!element || !property) return false;

	/* if value is undefined, get the previous value */
	if (value == undefined && element["_"+property] && element[property]) {
		value = element["_"+property];
	}

	/* define mirror hidden properties */
	Object.createHiddenElement(element, "_"+property, value);

	/* define public accessor */
	Object.defineProperty(element, property, {
		get : function(){
			var r = undefined;
			if (element._locked === false) {
				print("unlocked get("+property+")", element);
				element.__lock("plugin:"+property);
				r = getter ? getter.call(element) : undefined;
				element.__unlock("plugin:"+property);
			} else {
				print("locked get "+property, element);
			}
			return r == undefined ? element["_"+property] : r;
		},

		set : function(newValue){
			var oldValue = element["_"+property];

			if (newValue === oldValue || writable === false) {
				return false;
			} else {
				element["_"+property] = newValue;
			}

			if (element.loaded && element._locked === false) {
				print("set "+property+' = "'+newValue+'"', element);

				/* lock element */
				element.__lock("plugin:"+property);

				/* fire propertyupdate event if needed */
				DOMElement.onPropertyUpdate({
					element : element,
					property : property,
					oldValue : oldValue,
					newValue : newValue
				});

				/* optional user defined setter method */
				if (setter){
					print("plugin:set("+property+"="+newValue+")", element);
					var r = setter.call(element, newValue);
					if (r === false) {
						// handle readonly, restore old value
						element["_"+property] = oldValue;
						return false;
					}
				}

				/* unlock element */
				element.__unlock("plugin:"+property);
			}

		},

		enumerable : enumerable,
		configurable : configurable
	});
};

/* -------------------------------------------------------------------------- */

DOMElement.defineDescriptors = function(element, props){
	print("DOMElement.defineDescriptors", element);
	for (var key in props){
		if (props.hasOwnProperty(key)){
			var descriptor = props[key],
				value = element["_"+key] != undefined ?
						element["_"+key] : undefined;

			if (descriptor.value){
				if (typeof descriptor.value == "function"){
					value = descriptor.value.call(element);
				} else {
					value = descriptor.value;
				}
			}

			if (descriptor.writable === false){
				if (descriptor.setter || descriptor.getter){
					throw(element.type + '.js : Setter and Getter must not be defined for non writabe property "'+key+'".');
				}			
			}

			this.defineNativeProperty({
				element : element,
				property : key,
				value : value,

				setter : descriptor.set,
				getter : descriptor.get,

				writable : descriptor.writable,
				enumerable : descriptor.enumerable,
				configurable : descriptor.configurable
			});
		}
	}
};

/* -------------------------------------------------------------------------- */

DOMElement.definePublicProperties = function(element, props){
	for (var key in props){
		if (props.hasOwnProperty(key)){
			this.defineNativeProperty({
				element : element,
				property : key,
				value : props[key],
				enumerable : true,
				configurable : true
			});
		}
	}
};

DOMElement.defineReadOnlyProperties = function(element, props){
	for (var key in props){
		if (props.hasOwnProperty(key)){
			Object.createProtectedElement(element, key, props[key]);
		}
	}
};

DOMElement.defineInternalProperties = function(element, props){
	for (var key in props){
		if (props.hasOwnProperty(key)){
			Object.createHiddenElement(element, key, props[key]);
		}
	}
};

/* -------------------------------------------------------------------------- */


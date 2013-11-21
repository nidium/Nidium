/* ------------------------+------------- */
/* Native Framework 2.0    | Falcon Build */
/* ------------------------+------------- */
/* (c) 2013 nidium.com - Vincent Fontaine */
/* -------------------------------------- */

"use strict";

/* -------------------------------------------------------------------------- */

NDMElement.proxy = function(descriptor){
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
				element.__lock("plugin:"+property);
				r = getter ? getter.call(element) : undefined;
				element.__unlock("plugin:"+property);
			}
			return r == undefined ? element["_"+property] : r;
		},

		set : function(newValue){
			var oldValue = element["_"+property];

			if (newValue === oldValue || writable === false) {
				return false;
			} else {
				/* update element's hidden property */
				element["_"+property] = newValue;
			}

			if (element.initialized && element._locked === false) {
				/* lock element */
				element.__lock("plugin:"+property);

				/* update element's inline property container */
				element.inline[property] = newValue;

				/* fire propertyupdate event if needed */
				NDMElement.updater({
					element : element,
					property : property,
					oldValue : oldValue,
					newValue : newValue
				});

				/* optional user defined setter method */
				if (setter){
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

NDMElement.definePublicDescriptors = function(element, props){
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

			this.proxy({
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

NDMElement.defineDynamicProperties = function(element, props){
	for (var key in props){
		if (props.hasOwnProperty(key)){
			this.proxy({
				element : element,
				property : key,
				value : props[key],
				enumerable : true,
				configurable : true
			});
		}
	}
};

/* -------------------------------------------------------------------------- */

NDMElement.defineReadOnlyProperties = function(element, props){
	for (var key in props){
		if (props.hasOwnProperty(key)){
			Object.createProtectedElement(element, key, props[key]);
		}
	}
};

/* -------------------------------------------------------------------------- */

NDMElement.defineInternalProperties = function(element, props){
	for (var key in props){
		if (props.hasOwnProperty(key)){
			Object.createHiddenElement(element, key, props[key]);
		}
	}
};

/* -------------------------------------------------------------------------- */

/*

NDMElement.defineDynamicProperties(NDMElement.prototype, {
	id : null,
	className : "",

	left : 0,
	top : 0,

	width : 1,
	height : 1,

	innerWidth : -1,
	innerHeight : -1,

	scrollLeft : 0,
	scrollTop : 0,

	percentLeft : 0,
	percentTop : 0,
	percentWidth : 100,
	percentHeight : 100,

	offsetLeft : 0,
	offsetTop : 0,

	paddingLeft : 0,
	paddingRight : 0,
	paddingTop : 0,
	paddingBottom : 0,

	text : "",
	label : "",
	fontSize : 12,
	fontFamily : "arial",
	textAlign : "left",
	verticalAlign : "middle",
	textOffsetX : 0,
	textOffsetY : 0,
	lineHeight : 18,
	fontWeight : "normal",

	blur : 0,
	opacity : 1,
	alpha : 1,

	shadowBlur : 0,
	shadowColor : "rgba(0, 0, 0, 0.5)",
	shadowOffsetX : 0,
	shadowOffsetY : 0,

	textShadowBlur : 0,
	textShadowColor : '',
	textShadowOffsetX : 0,
	textShadowOffsetY : 0,

	color : '',
	background : '',
	backgroundImage : '',
	backgroundRepeat : true,
	radius : 0,
	borderWidth : 0,
	borderColor : '',
	outlineColor : 'blue',

	angle : 0,
	scale : 1,

	// -- misc flags
	canReceiveFocus : false,
	outlineOnFocus : true,
	canReceiveKeyboardEvents : false,

	visible : true,
	hidden : false,
	selected : false,
	overflow : true,
	multiline : false,
	editable : false,
	disabled : false,
	outline : false,
	
	scrollable : false,
	scrollBarX : false,
	scrollBarY : false,
	position : "relative",

	hover : false,
	hasFocus : false,

	cursor : "arrow"
});

*/
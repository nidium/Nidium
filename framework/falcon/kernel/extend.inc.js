/* ------------------------+------------- */
/* Native Framework 2.0    | Falcon Build */
/* ------------------------+------------- */
/* (c) 2013 Stight.com - Vincent Fontaine */
/* -------------------------------------- */

"use strict";

/* -------------------------------------------------------------------------- */

DOMElement.onPropertyUpdate = function(e){
	var element = e.element,
		old = e.oldValue,
		value = e.newValue;

	element.__lock();

	element.fireEvent("change", {
		property : e.property,
		oldValue : e.oldValue,
		newValue : e.newValue
	});

	switch (e.property) {
		case "left" :
		case "top" :
			element._needPositionUpdate = true;
			break;

		case "width" :
		case "height" :
			element._needSizeUpdate = true;
			break;

		case "opacity" :
			element._needOpacityUpdate = true;
			element._needRedraw = true;
			break;

		case "className" :
			element.updateProperties();
			element._needRedraw = true;
			break;

		default :
			element._needRedraw = true;
			break
	};

	element._needRefresh = true;
	element.__unlock();

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
				element.__lock();
				r = getter ? getter.call(element) : undefined;
				element.__unlock();
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
				/* lock element */
				element.__lock();

				/* fire propertyupdate event if needed */
				DOMElement.onPropertyUpdate({
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
				element.__unlock();

			}

		},

		enumerable : enumerable,
		configurable : configurable
	});
};

/* -------------------------------------------------------------------------- */

DOMElement.defineDescriptors = function(element, props){
	for (var key in props){
		if (props.hasOwnProperty(key)){
			var descriptor = props[key],
				value = undefined;

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


/* ------------------------+------------- */
/* Native Framework 2.0    | Falcon Build */
/* ------------------------+------------- */
/* (c) 2013 nidium.com - Vincent Fontaine */
/* -------------------------------------- */

"use strict";

/* -------------------------------------------------------------------------- */

document.getElements = function(){
	return this.layout.elements;
};

document.getElementsByName = function(name){
	return this.layout.find("name", name);
};

document.getElementsByTagName = function(name){
	return this.layout.find("type", name);
};

document.getElementsByClassName = function(className){
	var pattern = new RegExp("(^|\\s)"+className+"(\\s|$)"),
		z = this.layout.elements,
		elements = [];

	for (var i=0; i<z.length; i++){
		pattern.test(z[i]._className) && elements.push(z[i]);
	}

	elements.each = function(cb){
		if (typeof cb != "function") return false;
		for (var i in elements) {
			if (isNDMElement(elements[i])){
				cb.call(elements[i]);
			}
		}
	};

	return elements;
};

/* TODO : this */
document.getElementsBySelector = function(selector){
	var elements = [],
		l = selector.length,
		s = selector.substr(0, 1),
		p = s.in(".", "@", "#", "*") ? selector.substr(-(l-1)) : selector,
		m = p.split(":"),
		k = m[0],
		statefield = m[1];

	switch (s) {
		case "@" : /* static property container, do nothing */ break;
		case "#" : elements[0] = this.getElementById(k); break;
		case "." : elements = this.getElementsByClassName(k); break;
		default  : elements = this.getElementsByTagName(k); break;
	};

	if (statefield) {
		var temp = [],
			filters = statefield.split('+');  // UITextField:hover+disabled

		if (filters.length>0) {
			for (var i=0; i<elements.length; i++) {
				var checked = 0;
				for (var j=0; j<filters.length; j++) {
					var z = elements[i],
						state = filters[j];

					if (z && z[state]===true) checked++;
				}
				if (checked == filters.length) temp.push(elements[i]);
			}
		}
		elements = temp;
	}

	elements.each = function(cb){
		if (typeof cb != "function") return false;
		for (var i in elements) {
			if (isNDMElement(elements[i])){
				cb.call(elements[i]);
			}
		}
	};

	return elements;
};

document.getElementUnderPointer = function(){
	var element = null,
		x = window.mouseX,
		y = window.mouseY,
		z = this.layout.elements;

	for (var i=z.length-1 ; i>=0 ; i--) {
		if (z[i].layer.__visible && z[i].isPointInside(x, y)) {
			element = z[i];
			break;
		}
	}
	return element;
};

document.getElementById = function(id){
	var z = this.layout.elements,
		element = undefined;

	for (var i=0; i<z.length; i++){
		var o = z[i];
		if (o.id && o.id == id){
			element = z[i];
		}
	}
	return element;
};

/* -------------------------------------------------------------------------- */
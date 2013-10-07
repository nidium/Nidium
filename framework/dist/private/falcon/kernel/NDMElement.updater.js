/* ------------------------+------------- */
/* Native Framework 2.0    | Falcon Build */
/* ------------------------+------------- */
/* (c) 2013 nidium.com - Vincent Fontaine */
/* -------------------------------------- */

"use strict";

/* -------------------------------------------------------------------------- */

NDMElement.updater = function(e){
	var element = e.element,
		old = e.oldValue,
		value = e.newValue;

	element.__lock("updater");

	switch (e.property) {
		case "left" :
			element.layer.left = value;
			element.layer.scrollLeft = element._scrollLeft;
			element._needAncestorCacheClear = true;
			break;

		case "top" :
			element.layer.top = value;
			element.layer.scrollTop = element._scrollTop;
			element._needAncestorCacheClear = true;
			break;

		/*
		case "left" :
		case "top" :
			element._needPositionUpdate = true;
			element._needAncestorCacheClear = true;
			break;
		*/

		case "scrollLeft" :
		case "scrollTop" :
			element._needPositionUpdate = true;
			break;

		case "width" :
			element.layer.width = Math.round(value);
			element._needAncestorCacheClear = true;
			element._needRedraw = true;
			break;

		case "height" :
			element.layer.height = Math.round(value);
			element._needAncestorCacheClear = true;
			element._needRedraw = true;
			break;

		case "opacity" :
			element._needOpacityUpdate = true;
			break;


		case "id" :
		case "disabled" :
		case "hover" :
		case "selected" :
		case "className" :
			element.applyStyleSheet();
			element.applyInlineProperties();
			element._needOpacityUpdate = true;
			element._needPositionUpdate = true;
			element._needAncestorCacheClear = true;
			element._needSizeUpdate = true;
			element._needRedraw = true;
			break;

		case "position" :
			break;

		case "angle" :
			element.__resizeLayer();
			break;

		case "cursor" :
			/*
			if (element.isPointInside(window.mouseX, window.mouseY)){
				window.cursor = value;
			}
			*/
			break;

		default :
			element._needRedraw = true;
			break
	};

	element._needRefresh = true;

	element.__refresh();
	element.__unlock("updater");

	/* fire the propertyupdate event on element */
	element.fireEvent("propertyupdate", {
		property : e.property,
		oldValue : e.oldValue,
		newValue : e.newValue
	});

	/* call the user-defined element's update() method */
	element.update.call(element, {
		property : e.property,
		value : e.newValue
	});
};

/* -------------------------------------------------------------------------- */

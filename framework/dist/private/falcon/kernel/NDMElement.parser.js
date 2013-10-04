/* ------------------------+------------- */
/* Native Framework 2.0    | Falcon Build */
/* ------------------------+------------- */
/* (c) 2013 nidium.com - Vincent Fontaine */
/* -------------------------------------- */

"use strict";

/* -------------------------------------------------------------------------- */

NDMElement.parser = function(LST, callback){
	var createElement = function(node, parent){
		var element = null,
			nodeType = node.type,
			nodeAttributes = node.attributes;

		switch (node.type) {
			case "section" :
				nodeType = "UIElement";
				break;

			case "select" :
				nodeType = "UIDropDownController";
				break;

			case "option" :
				nodeType = "UIOption";
				break;

			case "view" :
				nodeType = "UIView";
				break;

			case "button" :
				nodeType = "UIButton";
				break;

			case "slider" :
				nodeType = "UISliderController";
				break;

			case "include":
				nodeType = null;
				break;

			default:
				break;
		}

		if (nodeType) {
			var element = parent.add(nodeType);
			element.setProperties(nodeAttributes);
		}
		return element;
	};

	var parseNodes = function(nodes, parent){
		for (var i=0; i<nodes.length; i++) {
			var node = nodes[i];
			if (node.type != "include") {
				var newParent = createElement(node, parent);
				parseNodes(node.children, newParent);
			}
		}
	};

	parseNodes(LST, document);
	if (typeof callback == "function") callback();
};

/* -------------------------------------------------------------------------- */
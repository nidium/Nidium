/* ------------------------+------------- */
/* Native Framework 2.0    | Falcon Build */
/* ------------------------+------------- */
/* (c) 2013 nidium.com - Vincent Fontaine */
/* -------------------------------------- */

"use strict";

/* -------------------------------------------------------------------------- */

NDMElement.listeners = {
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
			e.stopPropagation();
		}, false);

		element.addEventListener("mouseout", function(e){
			this.hover = false;
			e.stopPropagation();
		}, false);
	}
};

/* -------------------------------------------------------------------------- */

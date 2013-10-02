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
			if (this.disabled) {
				e.preventDefault();
				e.stopPropagation();
				return false;
			}
			this.selected = true;
		});

		element.addEventListener("mouseup", function(e){
			if (this.disabled) {
				e.preventDefault();
				e.stopPropagation();
				return false;
			}
			this.selected = false;
		});

		element.addEventListener("dragend", function(e){
			if (this.disabled) {
				e.preventDefault();
				e.stopPropagation();
				return false;
			}
			this.selected = false;
		});
	},

	addHovers : function(element){
		element.addEventListener("mouseover", function(e){
			if (this.disabled) {
				e.preventDefault();
				e.stopPropagation();
				return false;
			}
			this.hover = true;
			e.stopPropagation();
		}, false);

		element.addEventListener("mouseout", function(e){
			if (this.disabled) {
				e.preventDefault();
				e.stopPropagation();
				return false;
			}
			this.hover = false;
			e.stopPropagation();
		}, false);
	}
};

/* -------------------------------------------------------------------------- */

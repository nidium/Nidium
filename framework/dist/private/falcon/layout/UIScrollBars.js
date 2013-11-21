/* ------------------------+------------- */
/* Native Framework 2.0    | Falcon Build */
/* ------------------------+------------- */
/* (c) 2013 nidium.com - Vincent Fontaine */
/* -------------------------------------- */

/* -------------------------------------------------------------------------- */
/* NSS PROPERTIES                                                             */
/* -------------------------------------------------------------------------- */

document.nss.add({
	"UIScrollBar" : {
		background : "rgba(80, 80, 80, 0.0)",
		radius : 5
	},

	"UIScrollBarHandle" : {
		radius : 5,
		background : "rgba(18, 18, 18, 0.80)",
		shadowColor : "rgba(255, 255, 255, 0.15)",
		shadowBlur : 4
	}
});

/* -------------------------------------------------------------------------- */
/* ELEMENT DEFINITION                                                         */
/* -------------------------------------------------------------------------- */

Native.elements.export("UIScrollBar", {
	init : function(){
		var o = this.options;

		this.options = OptionalNumber(o.opacity, 0);
		this.hidden = OptionalBoolean(o.hidden, false);
		this.visible = false;

		this.addEventListener("dragstart", function(e){
			e.forcePropagation();
		}, true);

		this.addEventListener("drag", function(e){
			e.forcePropagation();
		}, true);

		this.addEventListener("dragend", function(e){
			e.forcePropagation();
		}, true);
	},

	draw : function(context){
		if (this.hidden) return false;
		var	params = this.getDrawingBounds();
		NDMElement.draw.box(this, context, params);
	}
});

/* -------------------------------------------------------------------------- */

Native.elements.export("UIScrollBarHandle", {
	init : function(){
		this.hidden = this.parent ? this.parent.hidden : false;
	},

	draw : function(context){
		if (this.hidden) return false;
		var	params = this.getDrawingBounds();

		NDMElement.draw.enableShadow(this);
		NDMElement.draw.box(this, context, params);
		NDMElement.draw.disableShadow(this);
	}
});

/* -------------------------------------------------------------------------- */

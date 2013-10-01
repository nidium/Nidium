/* ------------------------+------------- */
/* Native Framework 2.0    | Falcon Build */
/* ------------------------+------------- */
/* (c) 2013 nidium.com - Vincent Fontaine */
/* -------------------------------------- */

Native.elements.export("UIElement", {
	init : function(){
		this.canReceiveKeyboardEvents = false;
	},

	draw : function(context){
		var	params = this.getDrawingBounds();

		NDMElement.draw.softShadow(this);
		NDMElement.draw.box(this, context, params);
		NDMElement.draw.disableShadow(this);
	}
});
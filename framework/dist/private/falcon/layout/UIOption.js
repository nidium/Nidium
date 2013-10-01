/* ------------------------+------------- */
/* Native Framework 2.0    | Falcon Build */
/* ------------------------+------------- */
/* (c) 2013 nidium.com - Vincent Fontaine */
/* -------------------------------------- */

/* -------------------------------------------------------------------------- */
/* NSS PROPERTIES                                                             */
/* -------------------------------------------------------------------------- */

document.nss.add({
	"UIOption" : {
		label : "Default",
		fontSize : 11,
		fontFamily : "arial",

		paddingLeft : 8,
		paddingRight : 8,

		height : 22,
		radius : 0,
		background : "#ffffff",
		color : "#444444",

		value : "",
		disabled : false,

		cursor : "arrow"
	},

	"UIOption:hover" : {
		cursor : "pointer",
		background : "#444444",
		color : "#ffffff"
	},

	"UIOption:selected" : {
		background : "#4D90FE",
		color : "#ffffff",
		cursor : "arrow"
	},

	"UIOption:disabled" : {
		background : "#bbbbbb",
		color : "#dddddd",
		cursor : "arrow"
	},

	"UIOption:disabled+hover" : {
		background : "#aaaaaa",
		cursor : "arrow"
	}
});

/* -------------------------------------------------------------------------- */
/* ELEMENT DEFINITION                                                         */
/* -------------------------------------------------------------------------- */

Native.elements.export("UIOption", {
	init : function(){
		var self = this,
			o = this.options;

		this.addEventListener("contextmenu", function(e){
			e.preventDefault();
		}, false);

		NDMElement.listeners.addHovers(this);
	},

	draw : function(context){
		var	params = this.getDrawingBounds();
		NDMElement.draw.box(this, context, params);
		NDMElement.draw.label(this, context, params);
	}
});
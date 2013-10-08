/* ------------------------+------------- */
/* Native Framework 2.0    | Falcon Build */
/* ------------------------+------------- */
/* (c) 2013 nidium.com - Vincent Fontaine */
/* -------------------------------------- */

/* -------------------------------------------------------------------------- */
/* NSS PROPERTIES                                                             */
/* -------------------------------------------------------------------------- */

document.nss.add({
	"UIWindowResizer" : {
		width 	: 8,
		height 	: 8,
		left : null,
		top : null,
		right : 4,
		bottom : 4,
		cursor	: "arrow"
	},

	"UIWindowResizer:hover" : {
		cursor	: "pointer"
	}
});

/* -------------------------------------------------------------------------- */
/* ELEMENT DEFINITION                                                         */
/* -------------------------------------------------------------------------- */

Native.elements.export("UIWindowResizer", {
	init : function(){
		var o = this.options;

		this.layer.bottom = 4;
		this.layer.right = 4;

		this.updateElement = function(){
		};

		NDMElement.listeners.addDefault(this);

		this.addEventListener("drag", function(e){
			var win = this.parent;
			win.width += e.xrel;
			win.height += e.yrel;
		});
	},

	draw : function(context){
		var	params = this.getDrawingBounds();

        var x1 = params.x,
        	y1 = params.y,
        	x2 = params.x+params.w,
        	y2 = params.y+params.h;

		context.strokeStyle = "rgba(0, 0, 0, 0.3)";
        context.lineWidth = 1;

		context.beginPath();
		context.moveTo(x1, y2);
		context.lineTo(x2, y1);
		context.stroke();

		context.beginPath();
		context.moveTo(x1+4, y2);
		context.lineTo(x2, y1+4);
		context.stroke();

		context.strokeStyle = "rgba(255, 255, 255, 0.15)";
		context.beginPath();
		context.moveTo(x1+1, y2);
		context.lineTo(x2, y1+1);
		context.stroke();

		context.beginPath();
		context.moveTo(x1+5, y2);
		context.lineTo(x2, y1+5);
		context.stroke();
	}
});

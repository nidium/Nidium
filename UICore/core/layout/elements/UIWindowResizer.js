/* -------------------------- */
/* Native (@) 2012 Stight.com */
/* -------------------------- */

UIElement.extend("UIWindowResizer", {
	init : function(){
		this.w = this.options.w || 8;
		this.h = this.options.h || 8;
		this.x = this.parent.w - this.w - 4;
		this.y = this.parent.h - this.h - 4;

		this.addEventListener("mousedown", function(e){
			this.selected = true;
		});

		this.addEventListener("mouseup", function(e){
			this.selected = false;
			this.parent.scale = 1;
		});

		this.addEventListener("dragend", function(e){
			this.selected = false;
			this.parent.scale = 1;
		});

		this.addEventListener("mouseover", function(e){
			this.hover = true;
		});

		this.addEventListener("mouseout", function(e){
			this.hover = false;
		});

		this.addEventListener("drag", function(e){
			var win = this.parent;

			win.w += e.xrel;
			win.h += e.yrel;

			win.handle.w += e.xrel;
			win.handle.closeButton.left += e.xrel;

			win.contentView.w += e.xrel;
			win.contentView.h += e.yrel;
			this.left += e.xrel;
			this.top += e.yrel;
		});

	},

	draw : function(){
		this.x = this.parent.w - this.w - 4;
		this.y = this.parent.h - this.h - 4;

		var params = {
				x : this._x,
				y : this._y,
				w : this.w,
				h : this.h
			};


        var x1 = params.x,
        	y1 = params.y,
        	x2 = params.x+params.w,
        	y2 = params.y+params.h;

		canvas.strokeStyle = "rgba(0, 0, 0, 0.3)";
        canvas.lineWidth = 1;

		canvas.beginPath();
		canvas.moveTo(x1, y2);
		canvas.lineTo(x2, y1);
		canvas.stroke();

		canvas.beginPath();
		canvas.moveTo(x1+4, y2);
		canvas.lineTo(x2, y1+4);
		canvas.stroke();

		canvas.strokeStyle = "rgba(255, 255, 255, 0.15)";
		canvas.beginPath();
		canvas.moveTo(x1+1, y2);
		canvas.lineTo(x2, y1+1);
		canvas.stroke();

		canvas.beginPath();
		canvas.moveTo(x1+5, y2);
		canvas.lineTo(x2, y1+5);
		canvas.stroke();


	}
});

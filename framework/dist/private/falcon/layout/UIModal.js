/* ------------------------+------------- */
/* Native Framework 2.0    | Falcon Build */
/* ------------------------+------------- */
/* (c) 2013 Stight.com - Vincent Fontaine */
/* -------------------------------------- */

Native.elements.export("UIModal", {
	public : {
		width : {
			set : function(value){
				this.resizeElement();
			}
		},

		height: {
			set : function(value){
				this.resizeElement();
			}
		}
	},

	init : function(){
		var self = this,
			o = this.options;

		this.setProperties({
			opacity : 0,
			background : OptionalValue(o.background, "rgba(25, 25, 25, 0.80)")
		});

		this.left -= 10;
		this.top -= 10;
		this.width += 20;
		this.height += 20;

		this.hide();
		this.closable = false;

		this.addEventListener("mouseover", function(e){
			e.stopPropagation();
		}, false);

		this.addEventListener("mouseout", function(e){
			e.stopPropagation();
		}, false);

		this.addEventListener("mousedown", function(e){
			this.closable = true;
			e.stopPropagation();
		}, false);

		this.addEventListener("mouseup", function(e){
			if (this.closable){
				this.close();
				this.closable = false;
			}
			e.stopPropagation();
		}, false);

		this.addEventListener("mousewheel", function(e){
			e.stopPropagation();
		}, false);


		this.addEventListener("dragstart", function(e){
			e.stopPropagation();
		}, true);

		this.contentView = new UIView(this.parent, {
			opacity : 0,
			background : "#ffffff",
			width : this.width * 40/100,
			height : this.height * 40/100,
			radius : 3,
			shadowBlur : 22,
			shadowColor : "rgba(0, 0, 0, 0.80)",
			shadowOffsetY : 10,
			overflow : false,
			scrollable : true
		}).fix().center().hide();

		this.contentView.addEventListener("propertyupdate", function(e){
			if (e.property.in("width", "height")){
				this.center();
				modal.spinner.center();
			}
		});

		this.contentView.addEventListener("mousedown", function(e){
			e.stopPropagation();
		}, false);

		this.contentView.addEventListener("mouseup", function(e){
			e.stopPropagation();
		}, false);

		this.spinner = new UISpinner(this.contentView, {
			width : 24,
			height : 24,
			lineWidth : 3,
			dashes : 15,
			radius : 4,
			color : "#000000",
			opacity : 0.5
		}).fix().center().hide();

		this.open = function(){
			this.contentView.show();
			this.show();
			this.bringToFront();
			this.contentView.bringToFront();

			this.animate(
				"opacity", 0, 1,
				800, null,
				Math.physics.quadIn
			);
			this.contentView.fadeIn(150, function(){
				this.show();
			});
		};

		this.close = function(){
			this.fadeOut(75, function(){
				this.hide();
			});
			this.contentView.fadeOut(80, function(){
				this.hide();
			});
		};

		this.destroy = function(){
			this.contentView.remove();
			this.fadeOut(250, function(){
				this.remove();
			});
		};

		this.resizeElement = function(){
		};

		this.resizeElement();
	},

	draw : function(context){
		var	params = this.getDrawingBounds();
		DOMElement.draw.box(this, context, params);
	}
});

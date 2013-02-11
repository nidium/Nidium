/* ------------------------+------------- */
/* Native Framework 2.0    | Falcon Build */
/* ------------------------+------------- */
/* (c) 2013 Stight.com - Vincent Fontaine */
/* -------------------------------------- */

var main = new Application({id:"main"});

var addButton = new UIButton(main, {
	left : 140, 
	top : 185, 
	label : "Add"
});

addButton.hoverize(
	/* -- mouseover --- */
	function(e){
		var self = this;
		if (this.hinting || this.unhinting) return false;
		this.hinting = true;
		this.unhinting = false;

		var b = this.hint = new UIToolTip(addButton, {
			label : "Click here to see the magic"
		}).addEventListener("drag", function(e){
			t.left += e.xrel;
			t.top += e.yrel;
		});
		b.left = this.width;

		b.opacity = 0;
		b.fadeIn(250, function(){
			self.hinting = false;
		});
		b.slideX(this.width+18, 200, null, Math.physics.quadOut);

	},

	/* -- mouseout --- */
	function(e){
		var self = this,
			b = this.hint;

		if (this.hinting) {
			b.cancelCurrentAnimations("opacity");
			b.cancelCurrentAnimations("left");
			b.remove();
			this.hinting = false;
			this.unhinting = false;
		} else {
			this.unhinting = true;
			b.fadeOut(200, function(){
			b.unhinting = true;
				self.hinting = false;
				self.unhinting = false;
				this.remove();
			});
			b.slideX(this.width, 200, null, Math.physics.quadOut);
		}

	}
);


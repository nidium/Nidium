/* ------------------------+------------- */
/* Native Framework 2.0    | Falcon Build */
/* ------------------------+------------- */
/* (c) 2013 Stight.com - Vincent Fontaine */
/* -------------------------------------- */

Native.elements.export("UISpinner", {
	public : {
		speed : {
			set : function(value){
				this.speed = Number(value).bound(1, 100);
				this.refreshAnimation();
			}
		},

		width : {
			set : function(value){
				this.refreshLayout();
			}
		},

		height : {
			set : function(value){
				this.refreshLayout();
			}
		},

		radius : {
			set : function(value){
				this.refreshLayout();
			}
		},

		dashes : {
			set : function(value){
				this.refreshLayout();
			}
		},

		lineWidth : {
			set : function(value){
				this.refreshLayout();
			}
		}
	},

	init : function(){
		var self = this,
			o = this.options;

		this.setProperties({
			width : OptionalNumber(o.width, 20),
			height : OptionalNumber(o.height, 20),

			radius : OptionalNumber(o.radius, 2),
			dashes : OptionalNumber(o.dashes, 14),
			speed : OptionalNumber(o.speed, 30, 1, 100),
			lineWidth : OptionalNumber(o.lineWidth, 2.5),
			opacity : OptionalNumber(o.opacity, 0.95),

			background : OptionalValue(o.background, ""),
			color : OptionalValue(o.color, "#ffffff")
		});

		this.frame = 0;
		this.loop = true;
		this.playing = false;
		this.paused = false;
		this.forward = true;

		this.getOpacityArray = function(){
			var array = [],
				step  = 1 / this.dashes;

			for (var i=0; i<this.dashes; i++) array.push((i+1) * step);
			return array;
		};

		this.refreshLayout = function(){
			var width = this.lineWidth,
				height = Math.min(this.height/2, this.width/2),
				radius = Math.min(this.radius, height-width);

			this.layout = {
				opacities : this.getOpacityArray(),
				top : radius,
				left : 0-width/2,
				width : width,
				height : height - radius,
				radius : Math.min(height-radius, width) / 2
			};
		};

		this.refreshAnimation = function(){
			var ms = 1000/this.speed;
			if (this.timer) this.timer.remove();

			this.timer = Native.timer(function(){
				if (!self.playing) return false;

				if (self.forward) {
					self.nextFrame();
				} else {
					self.previousFrame();
				}

				//self._needRefresh = true;
				//self._needRedraw = true;
				self.redraw();
			}, ms, true, true);
		};

		this.play = function(){
			if (this.playing) return this;
			if (!this.paused) {
				this.refreshAnimation();
			}
			this.playing = true;
			this.paused = false;
		};

		this.pause = function(){
			if (!this.playing || this.paused) return this;
			this.playing = false;
			this.paused = true;
		};

		this.stop = function(){
			this.frame = 0;
			this.playing = false;
			this.paused = false;
			if (this.timer) this.timer.remove();
			return this;
		};

		this.destroy = function(){
			this.stop();
			this.remove();
		};

		this.nextFrame = function(){
			if ((++this.frame) >= this.dashes){
				if (this.loop){
					this.frame = 0;
				} else {
					this.stop();
				}
			}
			return this;
		};

		this.previousFrame = function(){
			if ((--this.frame) < 0){
				if (this.loop){
					this.frame = this.dashes-1;
				} else {
					this.stop();
				}
			}
			return this;
		};

		this.addEventListener("mouseover", function(e){
			e.forcePropagation();
		});

		this.addEventListener("mouseout", function(e){
			e.forcePropagation();
		});
	
		this.addEventListener("mousedown", function(e){
			e.forcePropagation();
		});

		this.addEventListener("mouseup", function(e){
			e.forcePropagation();
		});

		this.addEventListener("dragstart", function(e){
			e.forcePropagation();
		}, true);

		this.refreshLayout();
		this.refreshAnimation();
		this.play();
	},

	draw : function(context){
		var	params = this.getDrawingBounds(),
			layout = this.layout,

			scroll = function(array, d){
				if (!d) return array;
				return array.slice(d, array.length).concat(array.slice(0, d));
			},
		
			radian = function(degrees){
				return (degrees*Math.PI) / 180;
			},

			opacities = layout.opacities.scroll(-this.frame),
			rotation  = radian(360 / this.dashes);

		if (this.background){
			context.roundbox(
				params.x, params.y, 
				params.w, params.h, 
				4,
				this.background,
				false
			);
		}

		context.save();
		context.setColor(this.color);
		context.translate(this.width/2, this.height/2);

		for (var i=0; i < this.dashes; i++) {
			context.globalAlpha = opacities[i];
			context.fillRect(
				layout.left,
				layout.top,
				layout.width,
				layout.height,
				layout.radius
			);
			context.rotate(rotation);
		}

		context.restore();
	}
});

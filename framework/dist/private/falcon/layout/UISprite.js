/* ------------------------+------------- */
/* Native Framework 2.0    | Falcon Build */
/* ------------------------+------------- */
/* (c) 2013 nidium.com - Vincent Fontaine */
/* -------------------------------------- */

/* -------------------------------------------------------------------------- */
/* NSS PROPERTIES                                                             */
/* -------------------------------------------------------------------------- */

document.nss.add({
	"UISprite" : {
		width : 64,
		height : 64,
		speed : 10,
		min : 0,
		max : 1,
		value : 0,
		frame : 0
	}
});

/* -------------------------------------------------------------------------- */
/* ELEMENT DEFINITION                                                         */
/* -------------------------------------------------------------------------- */

Native.elements.export("UISprite", {
	update : function(e){
		if (e.property == "backgroundImage") {
			this.setBackgroundURL(e.value);
		}
		if (e.property == "value") {
			this.setValue(e.value);
		}
	},

	onAdoption : function(){
		this.resetAnimationTimer();
	},

	init : function(){
		var self = this,
			o = this.options;

		/* Element's Dynamic Properties */
		NDMElement.defineDynamicProperties(this, {
			speed : OptionalNumber(o.speed, 30),
			frame : OptionalNumber(o.frame, 0),
			min : OptionalNumber(o.min, 0),
			max : OptionalNumber(o.max, 100),
			value : OptionalNumber(o.value, 0)
		});

		this.loop = true;
		this.forward = true;
		this.playing = false;
		this.paused = false;
		this.loaded = false;
		this.orientation = 0;

		this.__animate = function(start, end, duration, callback, ease, fn){
			ease = ease || Math.physics.expoInOut;

			if (this.moving == true) {
				this.moving = false;
				this.finishCurrentAnimations("frame");
				this.frame = Math.round(end);
				this.checkFrame()
			} else {
				this.moving = true;

				this.__anim__ = this.animate("frame", start, end, duration,
					function(){
						self.moving = false;
						self.checkFrame();
						if (typeof(callback) == "function"){
							callback.call(self, self.value);
						}
					},

					ease,

					function(k){
						self.frame = Math.round(k);
						self.checkFrame();

						var value = self.frameToValue(self.frame);
						self.fireEvent("change", {
							value : value,
							frame : self.frame
						});

						if (typeof(fn) == "function"){
							fn.call(self, value);
						}
					}
				);
			}
		};

		this.stopCurrentAnimation = function(){
			if (this.__anim__) {
				var currentValue = this.valueToFrame(this.frame);
				this.__anim__.destroy();
				this.__anim__ = null;
				this.setValue(currentValue);
			}
		};

		this.valueToFrame = function(value){
			value = value.bound(this.min, this.max);
			var d = this.max - this.min; // TODO div by zero ???
			return Math.floor(frame = (value - this.min)*this.frames/d);
		};

		this.frameToValue = function(frame){
			frame.bound(0, this.frames-1);
			var d = this.max - this.min;
			return this.min + frame*d/this.frames;
		};

		this.setValue = function(value, duration, callback, ease, fn){
			this.__lock();
			this.value = value.bound(this.min, this.max);
			this.__unlock();

			var frame = this.valueToFrame(this.value);

			if (duration && duration>0 && this.frames) {
				this.__animate(this.frame, frame, duration, callback, ease, fn);
			} else {
				this.frame = Math.round(frame);
				this.checkFrame()
			}
		},

		this.setBackgroundURL = function(url){
			if (url) {
				this._cachedBackgroundImage = null;
				Image.load(url, function(img){
					self.setBackgroundImage(img);
				});
			}
		};

		this.setBackgroundImage = function(img){
			if (!img.width || !img.height) return false;
			var w = img.width,
				h = img.height;

			if (w<h) {
				if (w!=this.width) throw "Image width must be "+w;
				this.orientation = -1;
				this.frames = (h/this.height)|0;
			} else if (w == h) {
				this.orientation = 0
				this.frames = 1;
			} else {
				this.orientation = 1;
				this.frames = w/this.width;
			}

			this.sw = w;
			this.sh = h;
			this.setValue(this.value);

			if (!this.loaded && this.__onload){
				this.__onload.call(this);
			}

			this.loaded = true;
			this._cachedBackgroundImage = img;
			this._needRefresh = true;
			this._needRedraw = true;
			this.__refresh();
		};

		this.refreshLayout = function(){
			if (this.loaded === false) return false;
		};

		this.resetAnimationTimer = function(){
			if (this.speed === 0 || !this.playing) {
				return false;
			}
			var ms = 1000/this.speed;
			if (this.timer) this.timer.remove();

			this.timer = window.timer(function(){
				if (!self.playing) return false;

				if (self.forward) {
					self.nextFrame(1);
				} else {
					self.previousFrame(1);
				}
				self.redraw();
			}, ms, true, true);
		};

		this.play = function(){
			if (this.playing) return this;
			if (!this.paused) {
				this.resetAnimationTimer();
			}
			this.playing = true;
			this.paused = false;
			return this;
		};

		this.pause = function(){
			if (!this.playing || this.paused) return this;
			this.playing = false;
			this.paused = true;
			return this;
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

		this.checkFrame = function(){
			if (this.frame >= this.frames){
				if (this.loop){
					this.frame = 0;
				} else {
					this.frame = this.frames-1;
				}
			}

			if (this.frame < 0){
				if (this.loop){
					this.frame = this.frames-1;
				} else {
					this.frame = 0;
				}
			}
		};

		this.nextFrame = function(){
			this.frame++;
			this.checkFrame();
			return this;
		};

		this.previousFrame = function(){
			this.frame--;
			this.checkFrame();
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

		this.setBackgroundURL(this.backgroundImage);
	},

	draw : function(context){
		var	params = this.getDrawingBounds();

		NDMElement.draw.enableShadow(this);
		NDMElement.draw.box(this, context, params);
		NDMElement.draw.disableShadow(this);

		if (this.loaded && this._cachedBackgroundImage) {
			var img = this._cachedBackgroundImage,
				x = params.x,
				y = params.y,
				w = params.w,
				h = params.h,

				sx = (this.orientation ===  1) ? w*this.frame : 0,
				sy = (this.orientation === -1) ? h*this.frame : 0,
				sw = w,
				sh = h,

				dx = x,
				dy = y,
				dw = w,
				dh = h;

			if (this.orientation === -1 || this.orientation === 1) {
				// -1: vertical (i.e: 64 * 4096)
				//  1: horizontal (i.e: 4096 * 64) 
				context.drawImage(img, sx, sy, sw, sh, dx, dy, dw, dh);
			} else {
				// one unique frame
				context.drawImage(img, x, y);
			}

			/*
			context.save();
				context.clipbox(
					params.x-this._layerPadding, params.y-this._layerPadding,
					params.w, params.h,
					this.radius
				);
				context.clip();

				if (this.orientation === -1) {
					context.drawImage(
						this._cachedBackgroundImage,
						params.x, params.y-this.height*this.frame
					);
				} else if (this.orientation ===0){
					context.drawImage(
						this._cachedBackgroundImage,
						params.x, params.y
					);
				} else {
					context.drawImage(
						this._cachedBackgroundImage,
						params.x-this.width*this.frame, params.y
					);
				}
			context.restore();
			*/
		}

	}
});

/* ------------------------+------------- */
/* Native Framework 2.0    | Falcon Build */
/* ------------------------+------------- */
/* (c) 2013 nidium.com - Vincent Fontaine */
/* -------------------------------------- */

Native.elements.export("UIVideo", {
	init : function(){
		var self = this,
			o = this.options;

		this.setProperties({
			canReceiveFocus	: false,
			background : OptionalValue(o.background, 'black'),
			shadowBlur : OptionalNumber(o.shadowBlur, 12),
			shadowOffsetY : OptionalNumber(o.shadowOffsetY, 4),
			shadowColor : OptionalValue(o.shadowColor, "rgba(0, 0, 0, 1)"),
			overflow	: false
		});

		this.spinner = new UISpinner(this, {
			height : 40,
			width : 40,
			dashes : 12,
			lineWidth : 8,
			color : "white",
			speed : 16,
			opacity : 0.2,
			radius : 20
		}).center().move(0, -12).hide();

		this.status = new UIStatus(this, {
			progressBarColor : "rgba(210, 255, 60, 1)",
			progressBarLeft : 124,
			progressBarRight : 62,
			label : "",
			spinner : false,
			value : 0
		});

		this.status.hideDelay = 1000;

		this.status.icon = new Icon(this.status, {
			left : 0,
			top : 0,
			background : "rgba(0, 0, 0, 0.35)",
			color : "white",
			shape : "play"
		});

		this.status.icon.addEventListener("mousedown", function(e){
			var p = self.player;
			if (!p) return false;
			if (p.playing) {
				this.shape = "pause";
				p.pause();
			} else {
				this.shape = "play";
				p.play();
			}
		});

		this.status.speaker = new Icon(this.status, {
			left : 26,
			top : 0,
			color : "rgba(0, 0, 0, 0.55)",
			shape : "speaker",
			variation : 1
		});

		this.timecode = new UILabel(this.status, {
			left : this.width - 48,
			fontSize : 9,
			fontFamily : "time",
			top : 0,
			label : "00:00:00",
			color : 'rgba(255, 255, 255, 0.3)',
		});

		this.volume = new UISliderController(this.status, {
			left : 54,
			top : 6,
			width : 54,
			height : 9,
			background : '#161712',
			color : 'rgba(0, 0, 0, 1)',
			splitColor : 'rgba(80, 80, 80, 0.5)',
			boxColor : 'rgba(255, 255, 255, 0.02)',
			min : 0,
			max : 1.5,
			value : 0.8
		});

		this.status.isMouseOnProgressBar = function(e){
			var s1 = this.__left + 124,
				s2 = this.__left + this.width - 62;

			return (e.x >= s1 && e.x<= s2) ? true : false;
		};

		this.status.getSeekPosition = function(e){
			var s1 = this.__left + 124,
				s2 = this.__left + this.width - 62,
				max = s2 - s1,
				x = (e.x - s1),
				position = x/max * self.player.duration;

			return position;
		};


		this.status.addEventListener("mousemove", function(e){
			var p = self.player;
			if (!p) return false;

			if (this.isMouseOnProgressBar(e)) {
				this.cursor = "pointer";
				e.stopPropagation();
			} else {
				this.cursor = "arrow";
			}
		});

		this.status.addEventListener("mousedown", function(e){
			var p = self.player;
			if (!p) return false;

			if (this.isMouseOnProgressBar(e)) {
				if (p.playing) {
					p.pause();
					this.wasPaused = true;
				}

				self.seek(this.getSeekPosition(e));
				e.stopPropagation();
			}
		});

		this.status.addEventListener("mouseup", function(e){
			var p = self.player;
			if (!p) return false;

			if (this.isMouseOnProgressBar(e)) {
				this.cursor = "pointer";
				if (this.wasPaused) {
					p.play();
					this.wasPaused = false;
				}
				e.stopPropagation();
			} else {
				this.cursor = "arrow";
			}
		});


		this.status.addEventListener("dragstart", function(e){
			var p = self.player;
			if (!p) return false;
			this.dragging = true;
		});

		this.status.addEventListener("dragend", function(e){
			var p = self.player;
			if (!p) return false;
			this.dragging = false;
			if (this.wasPaused) {
				p.play();
				this.wasPaused = false;
			}
			if (this.isMouseOnProgressBar(e)) {
				this.cursor = "pointer";
				e.stopPropagation();
			}
		});

		this.status.addEventListener("drag", function(e){
			var s1 = this.__left + 124,
				s2 = this.__left + this.width - 62,
				max = s2 - s1,
				x = (e.x - s1),
				position = x/max * self.player.duration;

			if (e.x >= s1 && e.x<= s2) {
				self.seek(position);
				e.stopPropagation();
			}
		});

		this.addEventListener("mouseover", function(e){
			if (this.volume.draggingSlider || this.status.dragging) return false;
			if (!this.player || !this.player.ready) return false;

			this.mouseIsOver = true;
			this.mouseIsOut = false;

			clearTimeout(this.closingTimer);

			if (this.status.closing) {
				this.status.destroyCurrentAnimations("opacity");
				this.status.destroyCurrentAnimations("top");
				this.status.closing = false;
				this.status.closed = true;
				this.status.open(10);
			} else {
				this.status.open(150);
			}
		});

		this.addEventListener("mouseout", function(e){
			var that = this;
			if (this.volume.draggingSlider || this.status.dragging) return false;
			if (!this.player || !this.player.ready) return false;

			this.mouseIsOver = false;
			this.mouseIsOut = true;

			clearTimeout(this.closingTimer);

			this.closingTimer = setTimeout(function(){
				that.status.timering = false;
				if (that.mouseIsOver) {
					that.status.destroyCurrentAnimations("opacity");
					that.status.destroyCurrentAnimations("top");
					that.status.closing = false;
					that.status.closed = true;
					that.status.open(2000);
				} else {
					that.status.close(400);
				}
			}, this.status.hideDelay);
		});

		this.addEventListener("drag", function(e){
			this.left += e.xrel;
			this.top += e.yrel;
		});

		this.volume.addEventListener("change", function(e){
			if (!self.player) return false;
			self.player.volume = e.value;
			self.status.speaker.variation = Math.round(2*e.value/1.5);
		}, false);

		this.spinner.show();

		var toTimeCode = function(d){
			var f = Math.floor;
			d = Number(d)
			var h = f(d / 3600);
			var m = f(d % 3600 / 60);
			var s = f(d % 3600 % 60);

			return (h < 10 ? "0"+h : h) + ':' +
				   (m < 10 ? "0"+m : m) + ':' +
				   (s < 10 ? "0"+s : s);
		};

		this.seek = function(position){
			var dx = position - this.player.position;
			this.player.position += dx;
			this.status.value = Math.round(position*10000 / this.duration)/100;
			this.status.redraw();
			this.timecode.label = toTimeCode(position);
		};

		this.load = function(url, callback){
			var cb = OptionalCallback(callback, null);

			this.status.open();

			this.player = new VideoLayer(this.layer, url, function(e){
				self.duration = e.duration;
				cb.call(self, e);

				setTimeout(function(){
					self.spinner.fadeOut(600);
				}, 2000);

			});

			this.player.onplaying = function(e) {
				self.status.value = e.percent;
				self.status.redraw();
				self.timecode.label = toTimeCode(e.position);
			}


		};
	},

	draw : function(context){
		var	params = this.getDrawingBounds();

		if (this.shadowBlur != 0) {
			context.setShadow(
				this.shadowOffsetX,
				this.shadowOffsetY,
				this.shadowBlur,
				this.shadowColor
			);
		}

		NDMElement.draw.box(this, context, params);
		context.setShadow(0, 0, 0);
	}
});

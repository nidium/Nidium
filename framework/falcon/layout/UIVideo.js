/* ------------------------+------------- */
/* Native Framework 2.0    | Falcon Build */
/* ------------------------+------------- */
/* (c) 2013 Stight.com - Vincent Fontaine */
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
			overflow		: false
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

		this.addEventListener("mouseover", function(e){
			if (this.volume.draggingSlider) return false;
			if (!this.player || !this.player.ready) return false;
			this.status.open(200);
		});

		this.addEventListener("mouseout", function(e){
			if (this.volume.draggingSlider) return false;
			if (!this.player || !this.player.ready) return false;
			this.status.close(600);
		});

		this.addEventListener("drag", function(e){
			this.left += e.xrel;
			this.top += e.yrel;
		});

		this.status = new UIStatus(this, {
			progressBarColor : "rgba(210, 255, 60, 1)",
			progressBarLeft : 124,
			progressBarRight : 56,
			label : "",
			spinner : false,
			value : 0
		});

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
			left : this.width - 40,
			fontSize : 9,
			top : 1,
			label : "00:00:00",
			color : 'rgba(80, 80, 80, 0.5)',
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

		this.volume.addEventListener("update", function(value){
			if (!self.player) return false;
			self.player.volume = value;
			self.status.speaker.variation = Math.round(2*value/1.5);
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

		this.load = function(url, callback){
			var cb = OptionalCallback(callback, null);

			this.status.open();

			this.player = new VideoLayer(this.layer, url, function(e){
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

		DOMElement.draw.box(this, context, params);
		context.setShadow(0, 0, 0);
	}
});

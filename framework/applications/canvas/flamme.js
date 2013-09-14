/**
  * Burning Words, 
  * Effect of burning text with an ability to customize colors, 
  * fonts, burning speed and other parameters.
  *
  * Version: 1.0
  * Author: Michael Ryvkin, http://www.ponticstar.com
  * License: GNU Lesser General Public License, http://www.gnu.org/licenses/lgpl.html
  * Created: 2010-04-17
  * Last updated: 2010-04-17
  * Link: http://www.ponticstar.com/projects/burning-words/
  */

/* TODO */
/*

cast imageData width and heigt to int

*/

var ctx = Native.canvas.getContext("2d");

var BurningWords = function() {
	this.palette = null;
	this.canvas = null;

	this.stop = function() {
		
	};

	
	this.show = function(text, font_size){
		var bg_color = "000000",
			bg_alpha = 0;

		this.stop();

		// Define a palette
		if (this.palette === null) {
			this.palette = [];
			for (var i = 0; i < 64; i++) {
				this.palette[i] = [i * 4, 0, 0];
				this.palette[i + 64] = [255, i * 4, 0];
				this.palette[i + 128] = [255, 255, i * 4];
				this.palette[i + 192] = [255, 255, 255];
			}
		}

		this.text = text;
		this.text_color_string = "rgba(33, 0, 0, 1.0)";

		this.bg_color_r = 0;
		this.bg_color_g = 0;
		this.bg_color_b = 0;
		this.bg_color_a = 1;

		this.canvas = {};

		ctx.fontFamily = "times";
		ctx.fontSize = font_size;
		this.text_size = ctx.measureText(text).width;

		this.canvas.width = Math.round(this.text_size + 10);
		this.canvas.height = Math.floor(font_size*1.1 + 20);

		this.fire_decay = Math.floor(3 + Math.exp(3.6 - font_size / 7));

		this.image = ctx.createImageData(this.canvas.width, this.canvas.height);

		ctx.fillStyle = "#000000";
		ctx.fillRect(0, 0, this.canvas.width, this.canvas.height);
		this.image_flame = ctx.getImageData(0, 0, this.canvas.width, this.canvas.height);

		// drawing code here
		ctx.fillStyle = "#000000";
		ctx.fillRect(0, 0, this.canvas.width, this.canvas.height);

		ctx.fillStyle = "#ffffff";
		ctx.fontSize = font_size;
		ctx.fillText(text, 5, this.canvas.height/1.3);
		this.image_src = ctx.getImageData(0, 0, this.canvas.width, this.canvas.height);

		ctx.putImageData(this.image, 0, 0);

		var that = this;
		ctx.globalAlpha = 0.9;

		ctx.requestAnimationFrame(function(){
			that.burn();
		});

	};

	
	this.burn = function() {
		var image_data_len = this.canvas.width * this.canvas.height * 4;
		for (var pos = 0; pos < image_data_len; pos += 4) {
			if (this.image_src.data[pos] == 255) {
				this.image_flame.data[pos] = Math.floor(Math.random() * 256);
			}
		}

		var incr1 = (this.canvas.width * 4);
		for (var pos = 0; pos < (image_data_len - incr1); pos += 4) {
			var x = pos % incr1;

			var l = this.image_flame.data[((x == 0) ? pos + incr1 : pos) - 4];
			var r = this.image_flame.data[((x == incr1 - 4) ? pos - incr1 : pos) + 4];
			var b = this.image_flame.data[pos + incr1];
			var avg = Math.floor((l + r + b + b) / 4);

			// auto reduce it so you get lest of the forced fade and more vibrant fire waves
			if (avg > 0) {
				avg -= this.fire_decay;
			}

			// normalize
			if (avg < 0) {
				avg = 0;
			}

			this.image_flame.data[pos] = avg;
		}


		// 4 for 4 ints per pixel
		for (var pos = 0; pos < image_data_len; pos += 4) {
			if (this.image_flame.data[pos] != 0) {
				var c = this.image_flame.data[pos];
				var pal = this.palette;
				var a = (1 - (3 * c) / 255);
				if (a < 0) {
					a = 0;
				}
				if (a > this.bg_color_a) {
					a = this.bg_color_a;
				}
				this.image.data[pos] = Math.min(255, pal[c][0] + Math.floor(this.bg_color_r * a));
				this.image.data[pos + 1] = Math.min(255, pal[c][1] + Math.floor(this.bg_color_g * a));
				this.image.data[pos + 2] = Math.min(255, pal[c][2] + Math.floor(this.bg_color_b * a));
				this.image.data[pos + 3] = Math.max(this.bg_color_a, Math.min(3 * c, 255));

			} else {
				this.image.data[pos] = this.bg_color_r;
				this.image.data[pos + 1] = this.bg_color_g;
				this.image.data[pos + 2] = this.bg_color_b;
				this.image.data[pos + 3] = this.bg_color_a;
			}
		}

		ctx.fillStyle = "#000000";
		ctx.fillRect(0, 0, 1024, 768);

		ctx.putImageData(this.image, 80, 0);

		ctx.fillStyle = this.text_color_string;
		ctx.fillText(this.text, 80+5, this.canvas.height/1.3-1);
	}
};

var b = new BurningWords();
b.show("nidium", 90);




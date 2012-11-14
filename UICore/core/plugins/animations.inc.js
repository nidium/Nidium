/* -------------------------- */
/* Native (@) 2012 Stight.com */
/* -------------------------- */

DOMElement.implement({
	fadeIn : function(duration, callback, fx){
		this.animate("opacity", this.opacity, 1, duration, callback, fx);
	},

	fadeOut : function(duration, callback, fx){
		this.animate("opacity", this.opacity, 0, duration, callback, fx);
	},

	bounceScale : function(delta, duration, callback, fx){
		this.animate("scale", this.scale, delta, duration, callback, fx);
	},

	bounceBlur : function(delta, duration, callback, fx){
		this.animate("blur", this.blur, delta, duration, callback, fx);
	},

	slideX : function(delta, duration, callback, fx){
		this.animate("left", this.left, delta, duration, callback, fx);
	},

	scrollContentY : function(deltaY, callback){
		var self = this,

			startY = this.scroll.top,
			endY = self.scroll.top + deltaY,

			maxY = self.content.height - self.h,

			slice = 16,
			duration = 1200;
			time = 0;

		var stopScroll = function(){
			//self.scroll.initied = false;
			self.scroll.expired = true;
			echo("-------------------------------- self.scroll.expired")
			echo("Velocity :", time);
		};

		if (!self.scroll.initied) {
			self.scroll.initied = true;
			self.lastWheelEventTime = +new Date();
		} else {
			var now = +new Date(),
				time = now - self.lastWheelEventTime;

			self.scroll.expired = false;

			self.lastWheelEventTime = now;
			if (time>50) {
				time = 0;
				stopScroll();
				return false;
			} 

			clearTimeout(self.scroll.tim);
			delete(self.scroll.tim);

			self.scroll.tim = setTimeout(function(){
				stopScroll();
			}, 50);
		}

		if (!self.scroll.scrolling){
			self.scroll.scrolling = true;

			self.scroll.time = 0;
			self.scroll.duration = duration;

			if (deltaY>=0){
				echo('1 -------------------------', deltaY);
				self.scroll.startY = startY;
				self.scroll.endY = maxY - startY;
			} else {
				echo('2 -------------------------', deltaY);
				self.scroll.startY = startY;
				self.scroll.endY = -startY;
			}

			self.scroll.timer = setTimer(function(){
				let stop = false,
					value = Math.physics.cubicOut(
						0, 
						self.scroll.time, 
						self.scroll.startY, 
						self.scroll.endY, 
						duration
					);

				self.scroll.time += slice;

				if (deltaY>=0) {
					if (value > maxY) {
						value = maxY;
						stop = true;
					}
				}

				if (deltaY<0) {
					if (value <= 0) {
						value = 0;
						stop = true;
					}
				}

				if (self.scroll.time>duration){
					stop = true;
				}

				if (self.scroll.expired === true){
					stop = true;
				}

				if (stop && this.remove){
					echo("stopped");
					
					self.scroll.scrolling = false;
					self.scroll.initied = false;
					this.remove();
				} else {
					self.scroll.top = value;
				}

			}, slice, true, true);

		} else {
			/*
			self.scroll.timer.remove();
			self.scroll.scrolling = false;
			self.scroll.initied = false;
			*/
		}

		return false;

	},

	scrollY : function(deltaY){
		var self = this,
			startY = this.scroll.top,
			endY = this.scroll.top + deltaY,
			maxY = this.content.height - this.h,
			slice = 10,
			duration = 80;

		if (!this.scroll.initied){
			self.scroll.initied = true;
			self.scroll.time = 0;
			self.scroll.duration = duration;
			self.scroll.startY = startY;
			self.scroll.endY = endY;
			self.scroll.deltaY = deltaY;
			self.scroll.accy = 1.0;
		}

		if (this.scroll.scrolling) {
			self.scroll.timer.remove();
			this.scroll.scrolling = false;
			this.scroll.initied = false;
		}

		if (!this.scroll.scrolling){


			self.scroll.scrolling = true;

			self.scroll.timer = setTimer(function(){
				let stop = false;
		
				self.scroll.top = Math.physics.cubicOut(0, self.scroll.time, startY, deltaY, self.scroll.duration);
				self.scroll.time += slice;

				delete(self.__cache);
				canvas.__mustBeDrawn = true;


				if (deltaY>=0) {
					if (self.scroll.top > maxY) {
						self.scroll.top = maxY;
						stop = true;
					}

					if (self.scroll.top > endY) {
						self.scroll.top = endY;
						stop = true;
					}
				} else {
					if (self.scroll.top < endY) {
						self.scroll.top = endY;
						stop = true;
					}

					if (self.scroll.top < 0) {
						self.scroll.top = 0;
						stop = true;
					}
				}

				if (self.scroll.time>duration){
					stop = true;
				}

				if (stop && this.remove){
					self.scroll.scrolling = false;
					self.scroll.initied = false;
					this.remove();
				}

			}, slice, true, true);

		}
	},

	set : function(property, delta, duration, callback, fx){
		this.animate(property, this[property], delta, duration, callback, fx);
	},

	animate : function(property, from, delta, duration, callback, fx, rtCallback){
		Native.MotionFactory.add({
			view : this,
			property : property,
			from : from,
			delta : delta,
			duration : duration,
			callback : callback,
			rtCallback : rtCallback,
			fx : fx
		});
	}
});

Native.MotionFactory = {
	queue : [],
	timer : null,
	slice : 10,

	playing : false,
	ended : 0,

	add : function(o){
		if (o.view && o.view[o.property] != undefined){
			this.queue.push({
				view : o.view,
				property : String(o.property),
				start : Number(o.from),
				end : Number(o.delta - o.from),
				duration : Number(o.duration),
				callback : OptionalCallback(o.callback, null),
				rtCallback : OptionalCallback(o.rtCallback, null),
				fx : OptionalCallback(o.fx, Math.physics.quadInOut),
				time : 0,
				complete : false,
			});
		}
		this.play();
	},

	animate : function(animation){
		var view = animation.view,
			property = animation.property,
			duration = animation.duration,
			start = animation.start,
			end = animation.end;

		view[property] = animation.fx(0, animation.time, start, end, duration);
		if (animation.rtCallback) animation.rtCallback.call(view, view[property]);
		animation.time += this.slice;

		if (animation.time>duration){
			animation.complete = true;
			view[property] = start + end;
			if (animation.callback) animation.callback.call(view);
			delete(animation);
			canvas.__mustBeDrawn = true;
			this.ended++;
		}
	},

	play : function(){
		if (this.playing) return false;

		var self = this,
			q = this.queue;

		this.playing = true;
		this.ended = 0;

		this.timer = setTimer(function(){
			for (var i in q){
				if (!q[i].complete){
					self.animate(q[i]);
				}
			}
			canvas.__mustBeDrawn = true;

			if (self.ended>=q.length){
				self.playing = false;
				this.remove();
			}

		}, this.slice, true, true);

	}
};

/*
 * Open source under the BSD License. 
 * 
 * Copyright (c) 2008 George McGinley Smith
 * All rights reserved.
 * 
 * Redistribution and use in source and binary forms, with or without modification, 
 * are permitted provided that the following conditions are met:
 * 
 * Redistributions of source code must retain the above copyright notice, this list of 
 * conditions and the following disclaimer.
 * Redistributions in binary form must reproduce the above copyright notice, this list 
 * of conditions and the following disclaimer in the documentation and/or other materials 
 * provided with the distribution.
 * 
 * Neither the name of the author nor the names of contributors may be used to endorse 
 * or promote products derived from this software without specific prior written permission.
 * 
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY 
 * EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE
 * COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE
 * GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED 
 * AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
 * NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED 
 * OF THE POSSIBILITY OF SUCH DAMAGE. 
 *
*/

// t: current time, b: beginning value, c: change In value, d: duration

Math.physics = {
	def: 'quadOut',
	
	swing: function (x, t, b, c, d) {
		return Math.physics[Math.physics.def](x, t, b, c, d);
	},
	
	/* -- QUAD */
	quadIn: function (x, t, b, c, d) {
		return c*(t/=d)*t + b;
	},
	
	quadOut: function (x, t, b, c, d) {
		return -c *(t/=d)*(t-2) + b;
	},
	
	quadInOut: function (x, t, b, c, d) {
		if ((t/=d/2) < 1) return c/2*t*t + b;
		return -c/2 * ((--t)*(t-2) - 1) + b;
	},
	
	/* -- CUBIC */
	cubicIn: function (x, t, b, c, d) {
		return c*(t/=d)*t*t + b;
	},
	
	cubicOut: function (x, t, b, c, d) {
		return c*((t=t/d-1)*t*t + 1) + b;
	},
	
	cubicInOut: function (x, t, b, c, d) {
		if ((t/=d/2) < 1) return c/2*t*t*t + b;
		return c/2*((t-=2)*t*t + 2) + b;
	},
	
	/* -- QUART */
	quartIn: function (x, t, b, c, d) {
		return c*(t/=d)*t*t*t + b;
	},
	
	quartOut: function (x, t, b, c, d) {
		return -c * ((t=t/d-1)*t*t*t - 1) + b;
	},
	
	quartInOut: function (x, t, b, c, d) {
		if ((t/=d/2) < 1) return c/2*t*t*t*t + b;
		return -c/2 * ((t-=2)*t*t*t - 2) + b;
	},
	
	/* -- QUINT */
	quintIn: function (x, t, b, c, d) {
		return c*(t/=d)*t*t*t*t + b;
	},
	
	quintOut: function (x, t, b, c, d) {
		return c*((t=t/d-1)*t*t*t*t + 1) + b;
	},
	
	quintInOut: function (x, t, b, c, d) {
		if ((t/=d/2) < 1) return c/2*t*t*t*t*t + b;
		return c/2*((t-=2)*t*t*t*t + 2) + b;
	},

	/* -- Sinusoid */	
	sineIn: function (x, t, b, c, d) {
		return -c * Math.cos(t/d * (Math.PI/2)) + c + b;
	},
	
	sineOut: function (x, t, b, c, d) {
		return c * Math.sin(t/d * (Math.PI/2)) + b;
	},
	
	sineInOut: function (x, t, b, c, d) {
		return -c/2 * (Math.cos(Math.PI*t/d) - 1) + b;
	},
	
	/* -- exponential */
	expoIn: function (x, t, b, c, d) {
		return (t==0) ? b : c * Math.pow(2, 10 * (t/d - 1)) + b;
	},
	
	expoOut: function (x, t, b, c, d) {
		return (t==d) ? b+c : c * (-Math.pow(2, -10 * t/d) + 1) + b;
	},
	
	expoInOut: function (x, t, b, c, d) {
		if (t==0) return b;
		if (t==d) return b+c;
		if ((t/=d/2) < 1) return c/2 * Math.pow(2, 10 * (t - 1)) + b;
		return c/2 * (-Math.pow(2, -10 * --t) + 2) + b;
	},
	
	/* -- circular */
	circIn: function (x, t, b, c, d) {
		return -c * (Math.sqrt(1 - (t/=d)*t) - 1) + b;
	},
	
	circOut: function (x, t, b, c, d) {
		return c * Math.sqrt(1 - (t=t/d-1)*t) + b;
	},
	
	circInOut: function (x, t, b, c, d) {
		if ((t/=d/2) < 1) return -c/2 * (Math.sqrt(1 - t*t) - 1) + b;
		return c/2 * (Math.sqrt(1 - (t-=2)*t) + 1) + b;
	},
	
	/* -- Elastic */
	elasticIn: function (x, t, b, c, d) {
		var s=1.70158;var p=0;var a=c;
		if (t==0) return b;  if ((t/=d)==1) return b+c;  if (!p) p=d*.3;
		if (a < Math.abs(c)) { a=c; var s=p/4; }
		else var s = p/(2*Math.PI) * Math.asin (c/a);
		return -(a*Math.pow(2,10*(t-=1)) * Math.sin( (t*d-s)*(2*Math.PI)/p )) + b;
	},
	
	elasticOut: function (x, t, b, c, d) {
		var s=1.70158;var p=0;var a=c;
		if (t==0) return b;  if ((t/=d)==1) return b+c;  if (!p) p=d*.3;
		if (a < Math.abs(c)) { a=c; var s=p/4; }
		else var s = p/(2*Math.PI) * Math.asin (c/a);
		return a*Math.pow(2,-10*t) * Math.sin( (t*d-s)*(2*Math.PI)/p ) + c + b;
	},

	elasticInOut: function (x, t, b, c, d) {
		var s=1.70158;var p=0;var a=c;
		if (t==0) return b;  if ((t/=d/2)==2) return b+c;  if (!p) p=d*(.3*1.5);
		if (a < Math.abs(c)) { a=c; var s=p/4; }
		else var s = p/(2*Math.PI) * Math.asin (c/a);
		if (t < 1) return -.5*(a*Math.pow(2,10*(t-=1)) * Math.sin( (t*d-s)*(2*Math.PI)/p )) + b;
		return a*Math.pow(2,-10*(t-=1)) * Math.sin( (t*d-s)*(2*Math.PI)/p )*.5 + c + b;
	},
	
	/* -- Back */
	backIn: function (x, t, b, c, d, s) {
		if (s == undefined) s = 1.70158;
		return c*(t/=d)*t*((s+1)*t - s) + b;
	},
	
	backOut: function (x, t, b, c, d, s) {
		if (s == undefined) s = 1.70158;
		return c*((t=t/d-1)*t*((s+1)*t + s) + 1) + b;
	},
	
	backInOut: function (x, t, b, c, d, s) {
		if (s == undefined) s = 1.70158; 
		if ((t/=d/2) < 1) return c/2*(t*t*(((s*=(1.525))+1)*t - s)) + b;
		return c/2*((t-=2)*t*(((s*=(1.525))+1)*t + s) + 2) + b;
	},
	
	/* -- Bounce */
	bounceIn: function (x, t, b, c, d) {
		return c - Math.physics.bounceOut(x, d-t, 0, c, d) + b;
	},
	
	bounceOut: function (x, t, b, c, d) {
		if ((t/=d) < (1/2.75)) {
			return c*(7.5625*t*t) + b;
		} else if (t < (2/2.75)) {
			return c*(7.5625*(t-=(1.5/2.75))*t + .75) + b;
		} else if (t < (2.5/2.75)) {
			return c*(7.5625*(t-=(2.25/2.75))*t + .9375) + b;
		} else {
			return c*(7.5625*(t-=(2.625/2.75))*t + .984375) + b;
		}
	},
	
	bounceInOut: function (x, t, b, c, d) {
		if (t < d/2) return Math.physics.bounceIn(x, t*2, 0, c, d) * .5 + b;
		return Math.physics.bounceOut(x, t*2-d, 0, c, d) * .5 + c*.5 + b;
	}
};
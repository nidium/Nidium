/* ------------------------+------------- */
/* Native Framework 2.0    | Falcon Build */
/* ------------------------+------------- */
/* (c) 2013 nidium.com - Vincent Fontaine */
/* -------------------------------------- */

"use strict";

/* -------------------------------------------------------------------------- */

NDMElement.motion = {
	queue : {},
	uid : 0,
	nb : 0,

	timer : null,
	slice : 16,

	playing : false,
	ended : 0,

	get : function(o){
		var q = this.queue,
			property = String(o.property),
			view = o.view,
			animations = [];

		if (view && property) {
			for (var i in q){
				if (q.hasOwnProperty(i) && q[i].property == property){
					animations.push(q[i]);
				}
			}
		}
		return animations;
	},

	add : function(o){
		var self = this,
			animation = {},
			property = String(o.property);

		if (o.view && o.view[property] != undefined){
		
			if (o.view._mutex[property] === true) {
				o.view.finishCurrentAnimations(property);
			}

			o.view._mutex[property] = true;

			animation = {
				uid : "_anim_" + this.uid,
				view : o.view,
				property : property,
				start : Number(o.from),
				end : Number(o.delta - o.from),
				duration : Number(o.duration),
				callback : OptionalCallback(o.callback, null),
				rtCallback : OptionalCallback(o.rtCallback, null),
				fx : OptionalCallback(o.fx, Math.physics.quadInOut),
				time : 0,
				complete : false,
				finish : function(){
					self.finish(this);
				},
				destroy : function(){
					self.finish(this, false);
				}
			};

			this.queue[animation.uid] = animation;
			this.uid++;
			this.nb++;
		}
		this.play();
		return animation;
	},

	remove : function(animation){
		delete(this.queue[animation.uid]);
		this.nb--;
	},

	animate : function(animation){
		var view = animation.view,
			property = animation.property,
			duration = animation.duration,
			start = animation.start,
			end = animation.end;

		view[property] = animation.fx(0, animation.time, start, end, duration);

		if (animation.rtCallback) {
			animation.rtCallback.call(view, view[property]);
		}
		animation.time += this.slice;

		if (animation.time>=duration){
			view[property] = start + end;
			this.finish(animation);
			window.events.tick();
		}
	},

	finish : function(animation, callback=true){
		var	q = this.queue,
			view = animation.view,
			property = animation.property;

		animation.complete = true;
		view._mutex[property] = null;

		if (callback && animation.callback) animation.callback.call(view);
		this.remove(animation);

		this.ended++;
	},

	play : function(){
		if (this.playing) return false;
		var self = this,
			q = this.queue;

		this.playing = true;
		this.ended = 0;

		/*
		METHOD 1 --------------------------------------------------------
		*/
		var loop = function(time){
			var startTime = +new Date();
			for (var i in q){
				if (q.hasOwnProperty(i) && !q[i].complete){
					self.animate(q[i]);
				}
			}
			if (self.nb == 0){
				self.playing = false;
			}

			var execTime = (+new Date()) - startTime;
			self.slice = Math.max(16, execTime);

			if (self.playing) window.requestAnimationFrame(loop);
		};

		loop();

		/*
		METHOD 2 --------------------------------------------------------
		*/
		/*
		if (this.timer) this.timer.remove();
		this.timer = window.timer(function(){
			for (var i in q){
				if (q.hasOwnProperty(i) && !q[i].complete){
					self.animate(q[i]);
				}
			}

			if (self.nb == 0){
				self.playing = false;
				this.remove();
			}

		}, this.slice, true, true);
		*/
	}
};

NDMElement.implement({
	fadeIn : function(duration, callback, fx){
		this.animate("opacity", this.opacity, 1, duration, callback, fx);
	},

	fadeOut : function(duration, callback, fx){
		this.animate("opacity", this.opacity, 0, duration, callback, fx);
	},

	slideX : function(delta, duration, callback, fx){
		this.animate("left", this.left, delta, duration, callback, fx);
	},

	slideY : function(delta, duration, callback, fx){
		this.animate("top", this.top, delta, duration, callback, fx);
	},

	set : function(property, delta, duration, callback, fx){
		this.animate(property, this[property], delta, duration, callback, fx);
	},

	animate : function(property, from, delta, duration, callback, fx, rtCallback){
		return NDMElement.motion.add({
			view : this,
			property : property,
			from : from,
			delta : delta,
			duration : duration,
			callback : callback,
			rtCallback : rtCallback,
			fx : fx
		});
	},

	getCurrentAnimations : function(property){
		return NDMElement.motion.get({
			view : this,
			property : property
		});
	},

	finishCurrentAnimations : function(property){
		var q = this.getCurrentAnimations(property);

		for (var i in q){
			if (q.hasOwnProperty(i)){
				if (q[i].view == this) q[i].finish();
			}
		}
	},

	destroyCurrentAnimations : function(property){
		var q = this.getCurrentAnimations(property);

		for (var i in q){
			if (q.hasOwnProperty(i)){
				q[i].destroy();
			}
		}
	}
});

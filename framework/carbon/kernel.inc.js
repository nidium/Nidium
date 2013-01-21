/* -------------------------- */
/* Native (@) 2012 Stight.com */
/* -------------------------- */

/* -----------------------------------------------------------------------------
 * Native MTK : Native MultiTask Kernel                                        * 
 * ----------------------------------------------------------------------------- 
 * Version: 	1.0
 * Author:		Vincent Fontaine
 * 
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 * 
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 * 
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS
 * IN THE SOFTWARE.
 *
 */

const 
	__TASK_STATUS_IDLE__ = 0,
	__TASK_STATUS_RUNNING__ = 1,
	__TASK_STATUS_FREEZED__ = 2,
	__TASK_STATUS_COMPLETE__ = 3,
	__TASK_STATUS_ERROR__ = 4;


Native.scheduler = {
	tasks : [],
	nbtasks : 0,

	timer : null,
	clock : 16,

	qid : 0,
	currentWorker : null,

	_started : false,
	_paused : false,

	add : function(task){
		task.TID = this.nbtasks++;
		this.tasks.push(task);
	},

	start : function(){
		if (this._started) {
			return false;
		}
		this._started = true;
		this.cycle(); 
	},

	cycle : function(){
		var self = this;

		clearTimeout(this.timer);
		if (this._paused === false){
			this.timer = setTimeout(function(){
				self.switch();
			}, this.clock);
		}
	},

	findNext : function(){
		for (var t=0; t<this.tasks.length; t++){
			var task = this.tasks[t];
			if (task.TID !== this.qid && task.status){

			}
		}
	},

	switch : function(){
		var result, task, worker;

		if (this.qid >= this.tasks.length){
			this.qid = 0;
		}

		task = this.tasks[this.qid];

		if (task.status === __TASK_STATUS_RUNNING__){
			worker = task.worker;

			if (worker && "next" in worker 
				&& "close" in worker && "send" in worker){

				try {
					result = worker.next();
					task.cycle++;
				} catch (e) {
					worker.close();
					task.complete();
					this.oncomplete(task);
				}
			} else {
				task.error("Missing yield operator");
				this.oncomplete(task);
			}
		}
		
		/* 
		 * FIXME : when a task is complete, switch immediatly to the next
		 *         task, and do not wait for the next processing cycle.
		 */

		this.qid++;
		this.cycle();
	},

	freeze : function(){
		if (this._paused) return false;
		this._paused = true;
	},

	resume : function(){
		if (this._paused === false) return false;
		this._paused = false;
	},

	oncomplete : function(task){

	}
};

var Task = function(fn){
	var self = this;

	this.TID = 0;
	this.cycle = 0;
	this.priority = 1;
	this.status = __TASK_STATUS_RUNNING__;

	this.onfreeze = function(e){};
	this.onrelease = function(e){};
	this.oncomplete = function(e){};
	this.onerror = function(e){
		throw e.message + " (task "+e.tid+")";
	};

	this.freeze = function(){
		if (this.status !== __TASK_STATUS_RUNNING__) return false;
		this.onfreeze({
			tid : this.TID
		});
		this.status = __TASK_STATUS_FREEZED__;
	};

	this.release = function(){
		if (this.status !== __TASK_STATUS_FREEZED__) return false;
		this.onrelease({
			tid : this.TID
		});
		this.status = __TASK_STATUS_RUNNING__;
	};

	this.complete = function(){
		this.oncomplete({
			tid : this.TID
		});
		this.status = __TASK_STATUS_COMPLETE__;
	};

	this.error = function(message){
		this.onerror({
			message : message,
			tid : this.TID
		});
		this.status = __TASK_STATUS_ERROR__;
	};

	Native.scheduler.add(this);

	if (fn.toString().indexOf("yield") !== -1){
		this.worker = fn.apply(this);
		Native.scheduler.start();
	} else {
		this.error("Missing yield operator");
	}

};


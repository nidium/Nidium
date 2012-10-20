/* -------------------------- */
/* Native (@) 2012 Stight.com */
/* -------------------------- */

/* -----------------------------------------------------------------------------
 * SCTK : Simple Cooperative Threading Kernel                                  * 
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
	__TASK_STATUS_FINISHED__ = 2,
	__TASK_STATUS_FREEZED__ = 3;


Native.scheduler = {
	tasks : [],
	nbtasks : 0,

	timer : null,
	clock : 16,

	currentTID : 0,
	currentWorker : null,

	_started : false,

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
		this.timer = setTimeout(function(){
			self.next();
		}, this.clock);
	},

	next : function(){
		if (this.currentTID>=this.nbtasks) this.currentTID = 0;
		this.currentWorker = this.tasks[this.currentTID].worker();
		var result = this.currentWorker.next();
		
		if (result && result.next) {
			if (result) {
				result.next();
			} else {
				this.currentWorker.close();
			}
		}
		this.currentTID++;
		this.cycle();
	},

	stop : function(){
		if (!this._started) {
			return false;
		}
		this._started = false;
	}
}

function Task(fn){
	var self = this;

	this.TID = 0;
	this.cycle = 0;
	this.priority = 1;
	this.status = __TASK_STATUS_RUNNING__;

	this.worker = function(){
		while (self.status === __TASK_STATUS_RUNNING__) {
			yield fn.call(this, self.cycle++);
		}
	};

	Native.scheduler.add(this);
	Native.scheduler.start();
}

















/* -------------------------- */
/* Native (@) 2012 Stight.com */
/* -------------------------- */

var task0 = new Task(function(){
	for (var x=0; x<4; x++) {
		echo("task", this.TID, "said", x);
		yield;
	}
});


var task1 = new Task(function(){
	yield echo("task", this.TID, "said Step1");
	yield echo("task", this.TID, "said Step2");
	yield echo("task", this.TID, "said Step3");
	yield echo("task", this.TID, "said Step4");
	yield echo("task", this.TID, "said Step5");
	yield echo("task", this.TID, "said Step6");
	yield echo("task", this.TID, "said Step7");
});

var task2 = new Task(function(){
	echo("task", this.TID, "said nothing");
	yield;
});


var task3 = new Task(function(){
	var i = 0;
	while (true){
		yield echo("task", this.TID, i++);
		if (i===15) {
			this.freeze();
		}
	}
});

var task4 = new Task(function(){
	while (true){
		yield echo("task", this.TID, this.cycle);

		if (this.cycle === 40) task3.release();
		if (this.cycle === 80) Native.scheduler.pause();
	}
});


Native.scheduler.clock = 20; // switch tasks every 20 ms
Native.scheduler.pause();
Native.scheduler.resume();


/* Task Events */

task0.oncomplete = function(e){
	echo("***************** Task", e.tid, "complete (Task Event)");
};

task3.onerror = function(e){
	echo(e.message);
};

task3.onfreeze = function(e){
	echo("***************** Task ", e.tid, "freezed");
};

task3.onrelease = function(e){
	echo("***************** Task ", e.tid, "released");
};


/* Scheduler Events */

Native.scheduler.oncomplete = function(task){
	echo("***************** Task", task.TID, "complete (Scheduler Event)");
};




var k = 0;
setInterval(function(){
	echo("----------------- Generic Scope Timer", k++);
}, 500)


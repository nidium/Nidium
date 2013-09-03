/* ------------------------+------------- */
/* Native Framework 2.0    | Falcon Build */
/* ------------------------+------------- */
/* (c) 2013 nidium.com - Vincent Fontaine */
/* -------------------------------------- */

load("private/libs/mtk.lib.js");

var main = new Application({background : '#e0e0e0'});

var log = [];

log[0] = new UITextInput(main, {
	left : 10,
	top : 10,
	width : 998,
	height : 60,
	lineHeight : 12,
	fontSize : 10
});

log[1] = new UITextInput(main, {
	left : 10,
	top : 80,
	width : 242,
	height : 568,
	lineHeight : 12,
	fontSize : 10
});

log[2] = new UITextInput(main, {
	left : 262,
	top : 80,
	width : 242,
	height : 568,
	lineHeight : 12,
	fontSize : 10
});

log[3] = new UITextInput(main, {
	left : 514,
	top : 80,
	width : 242,
	height : 568,
	lineHeight : 12,
	fontSize : 10
});

log[4] = new UITextInput(main, {
	left : 766,
	top : 80,
	width : 242,
	height : 568,
	lineHeight : 12,
	fontSize : 10
});


function print(m, where){
	if (where) {
		where.append(m + "\n");
	} else {
		console.log(m);
	}
};


/* Define tasks */


var task0 = new Task(function(){
	for (var x=0; x<4; x++) {
		print("task " + this.TID + " said " + x, log[0]);
		yield;
	}
});

var task1 = new Task(function(){
	yield print("task " + this.TID + " said Step1", log[1]);
	yield print("task " + this.TID + " said Step2", log[1]);
	yield print("task " + this.TID + " said Step3", log[1]);
	yield print("task " + this.TID + " said Step4", log[1]);
	yield print("task " + this.TID + " said Step5", log[1]);
	yield print("task " + this.TID + " said Step6", log[1]);
	yield print("task " + this.TID + " said Step7", log[1]);
});

var task2 = new Task(function(){
	print("task " + this.TID + " start", log[2]);
	yield;
	yield;
	for (var z=0; z<20; z++){
		yield print("task " + this.TID + " said " + z, log[2]);
	}
});

var task3 = new Task(function(){
	var i = 0;
	while (true){
		yield print("task " + this.TID + " said " + i++, log[3]);
		if (i===12) {
			this.freeze();
		}
	}
});

var task4 = new Task(function(){
	while (true){
		yield print("task " + this.TID + " cycle ===" + this.cycle, log[4]);

		if (this.cycle === 30) task3.release();
		if (this.cycle === 45) Native.scheduler.freeze();
	}
});


Native.scheduler.clock = 50; // switch tasks every 20 ms
Native.scheduler.freeze();
Native.scheduler.resume();


/* Task Events */

task0.oncomplete = function(e){
	console.log("***************** Task 0 Complete (Task Event)");
};

task3.onerror = function(e){
	print(e.message, log[3]);
};

task3.onfreeze = function(e){
	print("***************** Task " + e.tid + " freezed", log[3]);
};

task3.onrelease = function(e){
	print("***************** Task " + e.tid + " released", log[3]);
};


/* Scheduler Events */

Native.scheduler.oncomplete = function(task){
	print("***************** Complete (Scheduler Event)", log[task.TID]);
};


/*

var k = 0;
setInterval(function(){
	console.log("----------------- Generic Scope Timer", k++);
}, 500)

*/

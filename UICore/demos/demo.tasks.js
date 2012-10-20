/* -------------------------- */
/* Native (@) 2012 Stight.com */
/* -------------------------- */


var task0 = new Task(function(cycle){
	echo("task", this.TID, "cycle", cycle, "said", 454);
});

var task1 = new Task(function(cycle){
	echo("task", this.TID, "cycle", cycle, "said foo");
});

var task2 = new Task(function(cycle){
	echo("task", this.TID, "cycle", cycle, "said bar");
});

var task3 = new Task(function(cycle){
	echo("task", this.TID, "cycle", cycle);
});


Native.scheduler.clock = 20; // switch tasks every 20 ms
Native.scheduler.pause();
Native.scheduler.release();


/*
var k = 0;
setInterval(function(){
	echo("----------------- Generic Scope Timer", k++);
}, 500)
*/

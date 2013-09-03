/* ------------------------+------------- */
/* Native Framework 2.0    | Falcon Build */
/* ------------------------+------------- */
/* (c) 2013 nidium.com - Vincent Fontaine */
/* -------------------------------------- */

/* Native Sandboxed Thread Demo */

/* Define a function that describe the job */
var job = function(params){
	for (var i=0; i<500000000; i++){
		if (i%20000000 == 0){
			this.send(params.k + i);
		}
	}
};

/* Create a new thread */
var t = new Thread(job);

/* add some listeners */
t.addEventListener("message", function(e){
	console.log(e.message);
});

t.addEventListener("complete", function(e){
	console.log("task complete");
});

t.addEventListener("error", function(e){
	console.log("error", e.message);
});

/* start the task */
t.start({
	id : 15,
	x : 320,
	k : 200
});


/* ------------------------+------------- */
/* Native Framework 2.0    | Falcon Build */
/* ------------------------+------------- */
/* (c) 2013 Stight.com - Vincent Fontaine */
/* -------------------------------------- */

var t = new Thread(function(){
	var p = 0;
	for (var i=0; i<500000; i++){
		if (i%10 == 0) this.send(i);
		p++;
	}
	return p;
});

t.onmessage = function(e){
	echo("new message", e.message);
};

t.oncomplete = function(e){
	echo("i'm done with", e.data);
};

t.start();


/* ---------------- */


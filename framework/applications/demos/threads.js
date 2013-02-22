/* ------------------------+------------- */
/* Native Framework 2.0    | Falcon Build */
/* ------------------------+------------- */
/* (c) 2013 Stight.com - Vincent Fontaine */
/* -------------------------------------- */

var t = Thread(...n){
	var p = 0;
	this.send(n);
	for (var i=0; i<5000000; i++){
		if (i%100 == 0) this.send(i);
		p++;
	}
	return n;
};

t.onmessage = function(e){
	var i = e.data;
	document.status.label = i;
	document.status.value = i*100/5000000;
};

t.oncomplete = function(e){
	if (e.data){
		echo("i'm done with", e.data);
	}
};

t.start(5, 6, 6, 9);


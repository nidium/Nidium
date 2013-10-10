/* ------------------------+------------- */
/* Native Framework 2.0    | Falcon Build */
/* ------------------------+------------- */
/* (c) 2013 nidium.com - Vincent Fontaine */
/* -------------------------------------- */

var t = new Thread(function(...n){
	var p = 0;
	for (var i=0; i<20000000; i++){
		if (i%10000 == 0) this.send(i);
		p++;
	}
	return n;
});

document.status.open();

t.onmessage = function(e){
	var i = e.data,
		v = i*100/20000000;

	document.status.label = Math.round(v)+"%";
	document.status.value = v;
};

t.oncomplete = function(e){
	if (e.data){
		console.log("i'm done with", e.data);
	}
	document.status.close();
};

t.start(5, 6, 6, 9);


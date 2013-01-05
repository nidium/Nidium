/* ------------------------+------------- */
/* Native Framework 2.0    | Falcon Build */
/* ------------------------+------------- */
/* (c) 2013 Stight.com - Vincent Fontaine */
/* -------------------------------------- */

echo("-- Low Level Asynchronous File Access Demo ---");

var f = new File("main.js");
f.onload = function(e){
	if (e.success) {
		var struct = {
			name : this.get(48, 7),
			product : this.get(16),
			build : this.get(74, 6)
		};
		for (var k in struct){
			echo(k, ":", struct[k].toString());
		}
		this.buffer.dump();
	}
};
f.load();



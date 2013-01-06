/* ------------------------+------------- */
/* Native Framework 2.0    | Falcon Build */
/* ------------------------+------------- */
/* (c) 2013 Stight.com - Vincent Fontaine */
/* -------------------------------------- */

/* -------------------------------------------------------------------------- */
load("libs/misc.lib.js");
/* -------------------------------------------------------------------------- */

/* -- High Level Asynchronous File Access -- */

File.getText("main.js", function(text){
	echo(text);
});

File.read("test.bin", function(buffer, size){
	buffer.dump();
});

File.write("test.txt", "new content", function(){
	echo("write successful");
});


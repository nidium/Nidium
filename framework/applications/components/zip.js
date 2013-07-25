/* ------------------------+------------- */
/* Native Framework 2.0    | Falcon Build */
/* ------------------------+------------- */
/* (c) 2013 Stight.com - Vincent Fontaine */
/* -------------------------------------- */

"use strict";

/* ---------------------------------------------------------------------------*/
load("libs/zip.lib.js");
/* ---------------------------------------------------------------------------*/

/*

File.read("test.zip", function(buffer, size){

	var zip = new ZipFile(buffer);
	var data = zip.read("test.txt");

	var z = new File("result.txt");
	z.open("w", function(){
		this.write(data, function(){
			this.close();
		});
	});
});

*/

File.unzip("test.zip", function(files){
	for (var file in files) console.log(file);
	this.extract("test.txt");
});



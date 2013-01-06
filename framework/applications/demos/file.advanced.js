/* ------------------------+------------- */
/* Native Framework 2.0    | Falcon Build */
/* ------------------------+------------- */
/* (c) 2013 Stight.com - Vincent Fontaine */
/* -------------------------------------- */

/* -------------------------------------------------------------------------- */
load("libs/misc.lib.js");
/* -------------------------------------------------------------------------- */

/* -- Low Level Asynchronous File Access -- */

var z = new File("test.bin");

z.open(function(){
	this.seek(4); // seek() is synchronous

	this.write("data5", function(){
		this.seek(2);
	
		var	buffer = malloc(2);

		buffer.data[0] = 0xf0;
		buffer.data[1] = 0xfe;

		this.write(buffer, function(){
			this.close();

			// read back, test.bin should contain 00 00 f0 fe 64 61 74 61 35
			File.read("test.bin", function(buffer, size){
				buffer.dump();
			});

		});
	});
});

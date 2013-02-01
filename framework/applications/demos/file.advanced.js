/* ------------------------+------------- */
/* Native Framework 2.0    | Falcon Build */
/* ------------------------+------------- */
/* (c) 2013 Stight.com - Vincent Fontaine */
/* -------------------------------------- */

/* -------------------------------------------------------------------------- */
require("libs/misc.lib.js");
/* -------------------------------------------------------------------------- */

/* -- Low Level Asynchronous File Access -- */

var z = new File("test.bin");

z.open("w+", function(){
	this.seek(4); // seek() is synchronous

	this.write("data5", function(){
		this.seek(2);
	
		var	buffer = malloc(2);

		buffer.data[0] = 0xf0;
		buffer.data[1] = 0xfe;

		this.write(buffer, function(){
			this.close();

			// read back using High Level API
			// test.bin should contain 00 00 f0 fe 64 61 74 61 35
			File.read("test.bin", function(buffer, size){
				buffer.dump();
			});

		});
	});
});


/*
 * File Modes
 *  
 * "r"	read: Open file for input operations. The file must exist.
 *
 * "w"	write:
 *		Create an empty file for output operations.
 *		If a file with the same name already exists, its contents are discarded and the file is treated as a new empty file.
 *
 * "a"	append:
 *		Open file for output at the end of a file.
 *		Output operations always write data at the end of the file, expanding it.
 *		Repositioning operations (seek, rewind) are ignored.
 *		The file is created if it does not exist.
 *
 * "r+"	read/update:
 *		Open a file for update (both for input and output).
 *		The file must exist.
 *
 * "w+"	write/update:
 *		Create an empty file and open it for update (both for input and output).
 * 		If a file with the same name already exists its contents are discarded
 *		and the file is treated as a new empty file.
 *
 * "a+"	append/update:
 *		Open a file for update (both for input and output) with all output
 *		operations writing data at the end of the file. Repositioning operations
 *		(seek, rewind) affects the next input operations, but output
 *		operations move the position back to the end of file.
 *		The file is created if it does not exist.
 *
 */ 
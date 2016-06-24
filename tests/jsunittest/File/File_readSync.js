/*
   Copyright 2016 Nidium Inc. All rights reserved.
   Use of this source code is governed by a MIT license
   that can be found in the LICENSE file.
*/

Tests.register("File.readSync", function(next) {
	var content = "123456789";
	var path = "File/doesexists/";
	var fileName = "simplefile.txt";
	var files = [ 
		"file://" + path + fileName,
					path + fileName,
    ];

	for (var i = 0; i < files.length; i++) {
		var file = new File(files[i], {encoding: "utf8"});
		var buffer = file.readSync();

		Assert.equal(buffer, content, 
                "Expected buffer to be \"" + content + "\" but got \"" + buffer + "\"");
	}
});

Tests.register("File.readSync (with read size)", function(next) {
	var content = "12345";
    var file = new File("File/doesexists/simplefile.txt", {encoding: "utf8"});
    var buffer = file.readSync(5);

		Assert.equal(buffer, content, 
                "Expected buffer to be \"" + content + "\" but got \"" + buffer + "\"");
});

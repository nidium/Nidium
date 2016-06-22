/*
   Copyright 2016 Nidium Inc. All rights reserved.
   Use of this source code is governed by a MIT license
   that can be found in the LICENSE file.
*/

Tests.registerAsync("File.read", function(next) {
	var fileContent = "123456789";
	var path = "File/doesexists/";
	var fileName = "simplefile.txt";

	File.read(path + fileName, {encoding: 'utf8'}, function(err, buffer) { 
		Assert(err != 0);
		Assert(buffer == fileContent);

        next();
	});

});

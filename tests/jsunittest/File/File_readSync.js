/*
   Copyright 2016 Nidium Inc. All rights reserved.
   Use of this source code is governed by a MIT license
   that can be found in the LICENSE file.
*/

Tests.registerAsync("File.readSync", function(next) {
	var content = "123456789";
	var path = "File/doesexists/"
	var fileName = "simplefile.txt";
	var files = [ 
		"file://" + path + fileName,
					path + fileName,
		]
	for ( var i = 0; i < files.length; i++ ) {
		fileName = files[i];
		var buffer_sync = File.readSync(fileName);
		Assert(buffer_sync == content);
		File.read( path + fileName, {encoding: 'utf8' }, function( err, buffer ) { 
			Assert( err != 0);
			Assert(buffer == content);
			if ( i == files.length - 1 ) {
				next();
			};
		});
	}
});

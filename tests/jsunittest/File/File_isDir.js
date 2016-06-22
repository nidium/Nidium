/*
   Copyright 2016 Nidium Inc. All rights reserved.
   Use of this source code is governed by a MIT license
   that can be found in the LICENSE file.
*/

Tests.register("File.isDir",  function() {
	var cases = {
	'.': true, 
	'./': true, 
	'..': true, 
	'../': true, 
	'../File': true, 
	'../File/': true, 
	'doesexists': true,
	'./doesexists': true,
	'./doesnotexist': false, 
	'doesnotexist': false, 
	'..': false		//notallowed
	};
for( var path  in cases) {
	var expected = cases[path];
	var file_object = new File(path);
	//console.log(JSON.stringify(file_object));
	Assert.equal(file_object.isDir(), expected);
}

});


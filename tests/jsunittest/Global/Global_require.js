/*
   Copyright 2016 Nidium Inc. All rights reserved.
   Use of this source code is governed by a MIT license
   that can be found in the LICENSE file.
*/

Tests.register("Global.require",  function() {
	load('Global/origional.js')
	Assert.equal(dummy_filename(), __dirname + 'origional.js');
	load('Global/symlink.js')
	Assert.equal(dummy_filename(), __dirname + 'symlink.js');
	//TODO: also for require
});

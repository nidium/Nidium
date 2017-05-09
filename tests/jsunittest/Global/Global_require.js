/*
   Copyright 2016 Nidium Inc. All rights reserved.
   Use of this source code is governed by a MIT license
   that can be found in the LICENSE file.
*/

Tests.register("Global.load",  function() {
    load('Global/dummy.js');
    Assert.equal(dummyFilename(), __dirname + 'dummy.js');
});

Tests.register("Global.load symlink",  function() {
    load('Global/dummy_symlink.js');
    Assert.equal(dummyFilename(), __dirname + 'dummy_symlink.js');
});

Tests.register("Global.require",  function() {
    var module = require('./dummy_symlink.js');
    // Require performs a "realpath()" operation on the symlink
    // So the expected filename returned is the symlinked file 
    Assert.equal(module.dummyFilename(), __dirname + 'dummy.js');
});


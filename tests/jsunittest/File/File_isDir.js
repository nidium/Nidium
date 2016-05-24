/*
   Copyright 2016 Nidium Inc. All rights reserved.
   Use of this source code is governed by a MIT license
   that can be found in the LICENSE file.
*/

Tests.register("File.isDir",  function() {
    var file;

    file = new File('.');
    console.log(JSON.stringify(file));
    Assert.equal(file.isDir(), true);

});


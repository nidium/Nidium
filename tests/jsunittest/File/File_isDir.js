/*
   Copyright 2016 Nidium Inc. All rights reserved.
   Use of this source code is governed by a MIT license
   that can be found in the LICENSE file.
*/

Tests.register("File.isDir",  function() {
    var cases = {
        '.': true, 
        './': true, 
        './File': true, 
        './File/': true, 
    };

    for(var path in cases) {
        var expected = cases[path];
        var fileObject = new File(path);

        fileObject.openSync("r");

        Assert.equal(fileObject.isDir(), expected, "Path : \"" + path + "\" should be a " + (cases[path] ? "directory" : "file"));
    }

});

Tests.register("File inexistent path", function() {
    Assert.throws(function() {
        var f = new File("does_not_exists");
        f.openSync("r");
    });
});

Tests.register("File outside root", function() {
    Assert.throws(function() {
        new File("..");
    });
});

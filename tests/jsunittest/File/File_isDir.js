/*
   Copyright 2016 Nidium Inc. All rights reserved.
   Use of this source code is governed by a MIT license
   that can be found in the LICENSE file.
*/

var IS_SERVER = typeof window == "undefined";

Tests.register("File.isDir",  function() {
    var cases = {
        '.': true, 
        './': true, 
    };

    if (!IS_SERVER) {
        cases["./File"] = true;
        cases["./File/"] = true;
    }

    for(var path in cases) {
        var expected = cases[path];
        var fileObject = new File(path);

        fileObject.openSync("r");

        Assert.equal(fileObject.isDir(), expected, "Path \"" + path + "\" should be a " + (cases[path] ? "directory" : "file"));
    }

});

Tests.register("File (inexistent path)", function() {
    Assert.throws(function() {
        var f = new File("does_not_exists");
        f.openSync("r");
    });
});

Tests.register("File (outside root)", function() {
    if (IS_SERVER) {
        var f = new File("..");
        f.openSync("r");
        Assert.equal(f.isDir(), true, "Path \"../\" should be a directory");
    } else {
        Assert.throws(function() {
            new File("..");
        });
    }
});

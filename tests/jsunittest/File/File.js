/*
   Copyright 2016 Nidium Inc. All rights reserved.
   Use of this source code is governed by a MIT license
   that can be found in the LICENSE file.
*/

var IS_SERVER = typeof window == "undefined";

Tests.register("File (remote URL exception)", function() {
    Assert.throws(function() {
        new File("http://www.nidium.com");
    });
});

Tests.register("File (inexistent file exception)", function() {
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

Tests.registerAsync("File.listFiles", function(next) {
    var expected = {
        "File": "dir",
        "JS": "dir",
        "unittests.nml": "file",
        "Thread": "dir",
        "manual_suites.js": "file"
    }

    var f = new File('.')

    f.listFiles(function(err, entries) {
        for (var name in expected) {
            var type = expected[name];
            var found = false;
            for( var i = 0; i < entries.length; i++) {
                if (entries[i].name == name && entries[i].type == type) {
                    console.log("Found " + type + " " + name);
                    found = true;
                    break;
                }
            }

            if (!found) {
                Assert("File " + name + " of type " + type + " not found");
            }
        }
        next();
    });
}, 1000);

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

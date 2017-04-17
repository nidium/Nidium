/*
   Copyright 2016 Nidium Inc. All rights reserved.
   Use of this source code is governed by a MIT license
   that can be found in the LICENSE file.
*/

var IS_SERVER = typeof window == "undefined";

// {{{ Constructor
Tests.register("File (remote URL exception)", function() {
    Assert.throws(function() {
        new File("http://www.nidium.com");
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
// }}}
// {{{ Properties
Tests.register("File.filename", function() {
    var f = new File(__filename);
    Assert.equal(f.filename, __filename,
            "Expected filename to be \"" + __filename + "\" but got " + f.filename + "\"");
});

Tests.register("File.filesize", function() {
    var f = new File("File/doesexists/simplefile.txt");
    f.openSync("r");
    Assert.equal(f.filesize, 9,
            "Expected filesize to be \"9\" but got \"" + f.filesize+ "\"");
});
// }}}

// {{{ Open
Tests.register("File.open (inexistent file exception)", function() {
    Assert.throws(function() {
        var f = new File("does_not_exists");
        f.open("r");
    });
});

Tests.registerAsync("File.open (writing)", function(next) {
    var f = new File("File/file_open_write_test");
    f.open("w+", function(err) {
        Assert.equal(err, undefined,
                "Got an error while trying to open file for writing : " + err);
        f.rm();
        next();
    });
});

Tests.registerAsync("File.open (reading)", function(next) {
    var f = new File(__filename);
    f.open("r", function(err) {
        Assert.equal(err, undefined,
                "Got an error while trying to open file " + __filename +
                " for reading : " + err);
        next();
    });
});

Tests.register("File.openSync (inexistent file exception)", function() {
    Assert.throws(function() {
        var f = new File("does_not_exists");
        f.openSync("r");
    });
});

Tests.register("File.openSync (writing)", function(next) {
    var f = new File("File/file_opensync_write_test");
    f.openSync("w+");
    f.rm();
});

Tests.register("File.openSync (reading)", function(next) {
    var f = new File(__filename);
    f.openSync("r");
});
// }}}

// {{{ Directory operation
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

Tests.register("File.mkdir", function() {

    var result = File.mkdir('File/dir_to_create');
    if (result === true) {

        var check = new File('File/dir_to_create');

        Assert.equal(check.isDir(), true, "Path \"File/dir_to_create\" should be a directory");

    }

});

Tests.register("File.rm", function() {
    var f = new File("File/file_to_rm_test");
    f.openSync("w+");
    f.rm();

    Assert.throws(function() {
        var f = new File("File/file_rm_test");
        f.open("r");
    });
});
// }}}

// {{{ Read
Tests.registerAsync("File.read", function(next) {
    var fileContent = "123456789";
    var f = new File("File/doesexists/simplefile.txt", {encoding:"utf8"});
    f.openSync("r");
    f.read(f.filesize, function(err, buffer) {
        Assert.equal(err, undefined, "Got an unexpected error while writing : " + err);
        Assert(buffer == fileContent);

        next();
    });
});

Tests.registerAsync("File.read (static)", function(next) {
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
// }}}

// {{{ Seek
Tests.registerAsync("File.seek", function(next) {
    var f = new File("File/doesexists/simplefile.txt", {encoding:"utf8"});
    f.openSync("r");
    f.seek(5, function(err) {
        Assert.equal(err, undefined, "Got an unexpected error while seeking : " + err);
        var content = f.readSync();
        Assert(content, "6789", "Got " + content + " but expected 6789");
        next();
    });
});
Tests.register("File.seekSync", function() {
    var f = new File("File/doesexists/simplefile.txt", {encoding:"utf8"});

    f.openSync("r");
    f.seekSync(5);

    var content = f.readSync();

    Assert(content, "6789", "Got " + content + " but expected 6789");
});
// }}}


// {{{ Write
Tests.registerAsync("File.write", function(next) {
    var content = "nidium";
    var f = new File("File/file_write_test", {encoding: "utf8"});
    f.openSync("w+");
    f.write(content, function(err) {
        Assert.equal(err, null, "Error while trying to write : " + err);

        f.seek(0, function() {
            var wroteContent = f.readSync();

            Assert.equal(f.filesize, content.length,
                    "Expected filesize to be " + content.length + " but got " + f.filesize);

            Assert.equal(wroteContent, content,
                    "Expected wrote content to be \"" + content + "\" but got \"" + wroteContent + "\"");
            f.rm();
            next();
        });
    });
});

Tests.register("File.writeSync", function(next) {
    var content = "nidium";
    var wroteContent = null;
    var f = new File("File/file_write_sync_test", {encoding: "utf8"});

    f.openSync("w+");
    f.writeSync(content);
    f.seekSync(0);

    wroteContent = f.readSync();

    Assert.equal(f.filesize, content.length,
            "Expected filesize to be " + content.length + " but got " + f.filesize);

    Assert.equal(wroteContent, content,
            "Expected wrote content to be \"" + content + "\" but got \"" + wroteContent + "\"");

    f.rm();
});

Tests.registerAsync("File.write (utf8)", function(next) {
    var content = "♥ nidium ♥";
    var f = new File("File/file_write_utf8_test", {encoding: "utf8"});
    f.openSync("w+");
    f.write(content, function(err) {
        Assert.equal(err, null, "Error while trying to write : " + err);

        f.seek(0, function() {
            var wroteContent = f.readSync();
            Assert.equal(wroteContent, content,
                    "Expected wrote content to be \"" + content + "\" but got \"" + wroteContent + "\"");
            f.rm();
            next();
        });
    });
});

Tests.register("File.writeSync (utf8)", function(next) {
    var content = "♥ nidium ♥";
    var wroteContent = null;
    var f = new File("File/file_write_sync_utf8_test", {encoding: "utf8"});

    f.openSync("w+");
    f.writeSync(content);
    f.seekSync(0);

    wroteContent = f.readSync();

    Assert.equal(wroteContent, content,
            "Expected wrote content to be \"" + content + "\" but got \"" + wroteContent + "\"");

    f.rm();
});

// }}}

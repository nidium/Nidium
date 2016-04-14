/*
   Copyright 2016 Nidium Inc. All rights reserved.
   Use of this source code is governed by a MIT license
   that can be found in the LICENSE file.
*/

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

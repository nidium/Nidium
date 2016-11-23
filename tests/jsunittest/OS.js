/*
   Copyright 2016 Nidium Inc. All rights reserved.
   Use of this source code is governed by a MIT license
   that can be found in the LICENSE file.
*/

Tests.register("OS.platform", function() {
    var OS = require("OS");

    Assert.equal(typeof OS.platform, "string", "OS.platform should be a string");
    Assert.notEqual(OS.platform, "unknown", "OS.platform should not be \"unknown\"");

    console.log(OS.platform);
});

Tests.register("OS.language", function() {
    var OS = require("OS");

    Assert.equal(typeof OS.language, "string", "OS.language should be a string");

    console.log(OS.language);
});

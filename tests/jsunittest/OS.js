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

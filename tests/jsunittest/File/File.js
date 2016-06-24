Tests.register("File (remote URL exception)", function() {
    Assert.throws(function() {
        new File("http://www.nidium.com");
    });
});

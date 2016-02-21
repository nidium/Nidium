// XXX : Obviously this script will fail to run on non en-us computer...
Tests.register("Navigator.language", function() {
    Assert.equal(navigator.language, 'en-us');
});


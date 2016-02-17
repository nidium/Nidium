Tests.register("System.getOpenFileStats", function() {
	var stats = System.getOpenFileStats();
	Assert.equal(stats.hasOwnProperty('cur'), true);
	Assert.equal(stats.hasOwnProperty('max'), true);
	Assert.equal(stats.hasOwnProperty('open'), true);
	Assert.equal(stats.hasOwnProperty('sockets'), true);
	Assert.equal(stats.hasOwnProperty('files'), true);
});


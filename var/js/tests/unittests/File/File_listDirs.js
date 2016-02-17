Tests.register("File.listFiles", function() {
	var f = new File('.')
	var expected = [	{name: "File", type: "dir"},
						{name: "JS", type: "dir"},
						{name: "unittests.nml", type:"file"},
						{name: "Thread", type:"dir"},
						{name: "manual_suites.js", type:"file"}];
	f.listFiles(function(err, entries) {
		for( var i = 0; i < entries.length; i++) {
			var entry = entries[i];
			//console.log(i  + " " + entry.name  + "  " + expected[i].name );
			Assert.equal(entry.name, expected[i].name);
			Assert.equal(entry.type, expected[i].type);
		}
	});
});


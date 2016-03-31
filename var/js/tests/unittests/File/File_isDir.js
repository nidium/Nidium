Tests.register("File.isDir",  function() {
	var file;

	file = new File('.');
	console.log(JSON.stringify(file));
	Assert.equal(file.isDir(), true);

});


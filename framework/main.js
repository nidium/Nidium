var stream = new Stream("http://p.nf/cafsde.mov");
stream.start(4096);
console.log("Filesize", stream.fileSize);

setInterval(function() {
	var data = stream.getNextPacket();
	if (data == null) {
		console.log("No data available");
		return;
	} else {

		//console.log("Got packet of size", data.byteLength);
	}
}, 500);

stream.onAvailableData = function() {
	console.log("New data is available", stream.fileSize);
	/*var i = 0;

	while (stream.getNextPacket() != null) {
		i++;
	}

	console.log("Read ", i, "packets");*/
}


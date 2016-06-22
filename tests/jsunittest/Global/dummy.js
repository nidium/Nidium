function dummyFilename() {
	return __filename;
}

if (typeof module != "undefined") {
    module.exports = {
        dummyFilename: dummyFilename
    }
}

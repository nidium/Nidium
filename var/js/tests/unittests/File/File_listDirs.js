var test_File_listFiles = function(){
	var f = new File('./dist')
	var success = false;
	var entries = f.listFiles(function(err, entry) {
		success = (entry == 'nidium-server' );
	});
	return success;
}

run_unit_tests = function( ) {
	return test_File_listFiles();
}

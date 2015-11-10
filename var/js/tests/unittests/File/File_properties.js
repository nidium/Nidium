var test_File_dirname = function(){
	var success = false;

	success = global.__dirname === global.pwd();
	return success;
}

run_unit_tests = function( ) {
	return test_File_dirname( );
}

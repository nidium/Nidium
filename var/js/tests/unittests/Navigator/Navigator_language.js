var test_Navigator_language = function(){
	var success = false;

	success = Navigator.language == 'en-us';

	return success;
}

run_unit_tests = function( ) {
	return test_Navigator_language();
}


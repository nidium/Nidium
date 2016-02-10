var test_JS_btoa = function(){
	var success = true;
	success = success && btoa("Hello Nidium") === "SGVsbG8gTmlkaXVt";
	success = success && btoa("hello nidium") === "aGVsbG8gbmlkaXVt";

	return success;
}

run_unit_tests = function( ) {
	return test_JS_btoa();
}


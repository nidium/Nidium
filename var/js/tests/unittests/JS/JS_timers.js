//Timeout
/*FIXME:
var test_JS_SetTimeoutString100 = function(){
	var success = true;
	if ( success ) {
		var t = setTimeout('console.log("Hello Nidium");');
		success = (t > 0);
	}

	return success;
}
*/

var test_JS_SetTimeoutFunction100 = function(){
	var success = true;
	if ( success ) {
		var t = setTimeout(function() {console.log("Hello Nidium");});
		success = (t > 0);
	}

	return success;
}


var test_JS_SetTimeoutFunction110 = function(){
	var success = true;
	if ( success ) {
		var t = setTimeout(function() {console.log("Hello Nidium");}, 1000);
		success = (t > 0);
	}

	return success;
}

var test_JS_SetTimeoutFunction111 = function(){
	var success = true;
	if ( success ) {
		var t = setTimeout(function(w) {console.log("Hello " + w);}, 1000, "nidium");
		success = (t > 0);
	}

	return success;
}

//Interval
/*FIXME
var test_JS_SetIntervalString100 = function(){
	var success = true;
	if ( success ) {
		var t = setInterval('console.log("Hello Nidium");');
		success = (t > 0);
	}

	return success;
}
*/

var test_JS_SetIntervalFunction100 = function(){
	var success = true;
	if ( success ) {
		var t = setInterval(function() {console.log("Hello Nidium");});
		success = (t > 0);
	}

	return success;
}


var test_JS_SetIntervalFunction110 = function(){
	var success = true;
	if ( success ) {
		var t = setInterval(function() {console.log("Hello Nidium");}, 1000);
		success = (t > 0);
	}

	return success;
}

var test_JS_SetIntervalFunction111 = function(){
	var success = true;
	if ( success ) {
		var t = setInterval(function(w) {console.log("Hello " + w);}, 1000, "nidium");
		success = (t > 0);
	}

	return success;
}

//clear
var test_JS_ClearTimeoutFunction111 = function(){
	var success = true;
	if ( success ) {
		var t = setInterval(function(w) {console.log("Hello " + w);}, 100000, "nidium");
		success = (t > 0);
		clearTimeout(t);
	}

	return success;
}

var test_JS_ClearIntervalFunction111 = function(){
	var success = true;
	if ( success ) {
		var t = setInterval(function(w) {console.log("Hello " + w);}, 100000, "nidium");
		success = (t > 0);
		clearInterval(t);
	}

	return success;
}

run_unit_tests = function( ) {
	return //test_JS_SetTimeoutString100() &&
		test_JS_SetTimeoutFunction100() &&
		test_JS_SetTimeoutFunction110() &&
		test_JS_SetTimeoutFunction111() &&

		//test_JS_SetIntervalString100() &&
		test_JS_SetIntervalFunction100() &&
		test_JS_SetIntervalFunction110() &&
		test_JS_SetIntervalFunction111() &&

		test_JS_ClearTimeoutFunction111() &&
		test_JS_ClearIntervalFunction111();
	}


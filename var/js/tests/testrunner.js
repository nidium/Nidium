TestRunner = function() {
	this.counters = {
		suites: 0,
		success: 0,
		fails: 0
	};
	this.report = function() {
		console.log("\nResult:\n\ttests: " + this.counters.suites + "\n\tsuccess: " + this.counters.success +"\n\tfails: " + this.counters.fails + "\n")
		if (this.counters.success == this.counters.suites && this.counters.fails == 0 && this.counters.success > 0) {
			console.log("YES: all test were OK!");
		} else {
			console.log("NOT All tests were ok! Go fix them");
		}
	};
	this.run_test_suite = function(include_name) {
		try {
			this.counters.suites++;
			require(include_name);
			success = run_unit_tests();
			if (success) {
				console.log(include_name + ": OK");
				this.counters.success++;
			} else {
				console.log(include_name + ": did not work correctly");
				this.counters.fails++
			}
		} catch (err) {
			console.log(include_name + ": could not be loaded: \n\t\t'" + err.message + "'");
			this.counters.fails++
		}
	};
	this.do_test_suites = function(suites) {
		for (var i = 0; i < suites.length; i++) {
			this.run_test_suite(suites[i]);
		}
		get_test_suites = null;
	}
	return this;
}


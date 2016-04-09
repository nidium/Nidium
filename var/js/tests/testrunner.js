// Make Assert object global for ease of use during the tests
Assert = require("./assert");
var log = {
    error: function(...args) {
        args.unshift(31);
        log._raw(...args);
    },
    info: function(...args) {
        args.unshift("38;5;33");
        log._raw(...args);
    },
    warning: function(...args) {
        args.unshift(35);
        log._raw(...args);
    },
    success: function(...args) {
        args.unshift(32);
        log._raw(...args);
    },
    print: function(...args) {
        console.write(...args);
    },
    _raw: function(...args) {
        var color = args.shift();
        args = args.join(" , ");
        console.write("\x1b[" + color + "m" + args + "\x1b[0m");
    }
}

TestsRunner = function() {
    this.tests = [];
    this.lastFile = null;
    this.counters = {
        "fails": 0,
        "success": 0,
        "total": 0,
        "suites": 0
    };
    this.failedTests = [];

    this.lastException = {
        "trace": [],
        "exception": null
    };

    this.testWatchdogTimer = -1;
    this.currentTest = null;

    this.currentFile = null;
    this.completionCallback = null;
};

TestsRunner.prototype = {
    reportLastException: function() {
            log.error("Exception : " + this.lastException.exception + "\n");
            var regex = new RegExp( '^' + pwd() + '[^/]+$');
            for (var i = 0; i < this.lastException.trace.length; i++) {
                var frame = this.lastException.trace[i];
                // This is not 100% ideal, but the noisyness in the stacktrace is reduced
                if (!regex.exec(frame.script)) {
                    var scriptShort = "\t" + i + " ../" + frame.script.split("/").slice(-3).join("/");
                    console.write(scriptShort + ":" + frame.line + " " + frame.callee + "()\n");
                }
            }
    },

    initDebugger: function() {
        // Debugger is catching async exception/assert
        // synchronous exception/assert are handled in a regular try/catch
        this.dbgCtx = Debugger.create();

        Debugger.run(this.dbgCtx, function(dbg, testRunner) {
            /*
            dbg.onEnterFrame = function(frame) {
                console.log("enter frame : " + frame.script.url + ":" + frame.script.getOffsetLine(frame.offset));
                // For newer version of SM
                // if (typeof script.getOffsetLocation == "function") {
                //  line = script.getOffsetLocation(state.offsets[i]).lineNumber;
                // } else {
                // }
            }
            */

            var oldFrame = null;

            function parseFrame(frame) {
                var frameName = frame.callee && frame.callee.displayName ? frame.callee.displayName : "anonymous";
                return {
                    "callee": frameName,
                    "script": frame.script.url,
                    "line": frame.script.getOffsetLine(frame.offset)
                }
            }

            dbg.onExceptionUnwind = function(frame, value) {
                // Older frame is different from last frame
                // the exception has been intercepted by a try/catch
                // Reset the stack trace
                if (oldFrame != frame) {
                    testRunner.lastException.trace = [];
                }
                oldFrame = frame.older;

                // onExceptionUnwind is called for each frame
                // Use this to build a stacktrace of the error
                testRunner.lastException.trace.push(parseFrame(frame));
                testRunner.lastException.exception = 
                    value ? value.unsafeDereference() : "No exception error :(";

                if (frame.older == null) {
                    // We have reached the last frame
                    // report the failure, and run the next test
                    testRunner._reportTest(false);
                    
                    // Prevent the exeception from bubbling up
                    return {"return": 42};
                }
            }
        }, this);
    },

    register: function(name, fn, watchdogDelay=0, async=false) {
        this.tests.push({
            "name": name, 
            "function": fn, 
            "file": this.currentFile,
            "watchdogDelay": watchdogDelay, 
            "async": async
        });
        this.counters.total++;
    },

    registerAsync: function(name, fn, watchdogDelay) {
        this.register(name, fn, watchdogDelay, true);
    },

    load: function(suites) {
        log.info("Loading tests...\n");
        for (var i = 0; i < suites.length; i++) {
            var includeName = suites[i];
            console.write("- " + includeName);

            this.currentFile = includeName;
            this.counters.suites++;

            // Expose to tests the current instance of the TestsRunner
            Tests = this;

            try {
                load(includeName);
            } catch(e) {
                log.error(e);
                process.exit(2)
            }

            log.success("\t OK\n");
        }

        return true;
    },

    _reportTest: function(success) {
        clearTimeout(this.testWatchdogTimer);

        if (success) {
            log.success("\r[ SUCCESS ]");
            console.write("\n");
            this.counters.success++;
        } else {
            log.error("\r[  ERROR  ]");
            console.write("\n");
            this.reportLastException();
            // XXX : Should we exit here ? 
            this.counters.fails++;
            this.failedTests.push(this.currentTest);
        }

        this.currentTest = null;

        setImmediate(this._nextTest.bind(this));
    },

    _nextTest: function() {
        var test = this.tests.shift();
        this.currentTest = test;

        if (!test) {
            if (this.completionCallback) this.completionCallback();
            return;
        }

        if (test.file != this.lastFile) {
            console.write("-----------------------------\n");
            log.info("Running tests for " + test.file + "\n");
            console.write("-----------------------------\n");
            this.lastFile = test.file;
        }

        console.write("[ RUNNING ] " + test["name"]);
        console.write("\n");

        /*
         * Tests are ran in a setImmediate() call, so _nextTest() will be the
         * last visible frame to the debugger. As the only way for the debugger
         * to know that an exception hasn't been catched is to check if the 
         * frame is the last frame on the stack, we run the test inside a closure.
         */
        var success = false;
        (function() {
            try {
                test["function"](function() {
                    this._reportTest(true);
                }.bind(this));
                success = true;
            } catch (err) {
                sucess = false;
            }
        }.bind(this))();

        if (!test.async || success === false) {
            this._reportTest(success);
        } else if (test.watchdogDelay) {
            this.testWatchdogTimer = setTimeout(function() {
                Assert(false, "Timeout reached");
            }, test.watchdogDelay);
        }
    },

    run: function(callback) {
        this.initDebugger();

        this.completionCallback = callback;

        this._nextTest();
    },

    report: function(exit=false) {
        var success = this.counters.success == this.counters.total && this.counters.fails == 0;
        var reportLog = success ? log.success : log.error;

        if (success) {
            reportLog("\n+-----------P A S S E D------------+\n");
        } else {
            reportLog("\n+-----------F A I L E D-------------+\n");
        }

        reportLog("|");
        reportLog("\r\033[35C |\n");
        reportLog("| Tests\t\t: " + this.counters.total + " (" + this.counters.suites + " suites)");
        reportLog("\r\033[35C |\n");
        reportLog("| Success\t: " + this.counters.success);
        reportLog("\r\033[35C |\n");
        reportLog("| Failed\t: " + this.counters.fails);
        reportLog("\r\033[35C |\n");
        reportLog("|");
        reportLog("\r\033[35C |\n");
        reportLog("+-----------------------------------+\n");

        if (this.failedTests.length > 0) {
            reportLog("Failed tests : ");
            for (var i = 0; i < this.failedTests.length; i++) {
                try {
                    console.write(" - " + this.failedTests[i].name + "\n");
                } catch (e) {
                    log.info(e);
                }
            }
        }

        if (exit) {
            process.exit(success ? 0 : 1);
        }

        return success;
    }
}

module.exports = TestsRunner;


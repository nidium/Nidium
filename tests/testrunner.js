/*
   Copyright 2016 Nidium Inc. All rights reserved.
   Use of this source code is governed by a MIT license
   that can be found in the LICENSE file.
*/

// Make Assert object global for ease of use during the tests
Assert = require("./assert");
var OS = require("OS");

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

function clear() {
    // Remove any added canvas
    var childrens = document.canvas.getChildren();
    for (children of childrens) {
        children.removeFromParent();
    }

    // Clear the root canvas
    document.canvas.ctx.clearRect(0, 0, document.canvas.width, document.canvas.height);
}

function sanitize(str) {
    return str.replace(/[^a-zA-Z0-9-]/, "_");
}

function shot(path) {
    var canvas = document.toDataArray();
    var f = new File(path);

    f.openSync("w+");
    f.writeSync(canvas.data.buffer);
    f.closeSync();
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
    this.visualTests = {};
    this.hasVisualTests = false;

    this.visualTestsReport = "";

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
            var regex = new RegExp( '^' + process.cwd() + '[^/]+$');
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
        this.dbgCtx = new DebuggerCompartment();

        this.dbgCtx.run(function(dbg, testRunner) {
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

    register: function(name, fn, watchdogDelay=0, async=false, visual=false) {
        if (this.testRegex && !this.testRegex.test(name)) {
            return;
        }

        this.tests.push({
            "name": name, 
            "function": fn, 
            "file": this.currentFile,
            "watchdogDelay": watchdogDelay, 
            "async": async,
            "visual": visual
        });

        this.counters.total++;
    },

    registerAsync: function(name, fn, watchdogDelay) {
        this.register(name, fn, watchdogDelay, async=true);
    },

    registerVisual: function(name, fn, watchdogDelay) {
        this.register(name, fn, watchdogDelay, async=true, visual=true);
    },

    setTestRegex: function(regex) {
        if (!(regex instanceof RegExp)) {
            regex = new RegExp(regex);
        }
        this.testRegex = regex;
    },

    setSaveVisualDiff: function(enabled) {
        this.saveVisualDiff = enabled;
    },

    load: function(suites, regex) {
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

    _reportTest: function(success, visual=false) {
        clearTimeout(this.testWatchdogTimer);

        if (success) {
            log.success("\r[ SUCCESS ]");
            console.write("\n");
            if (!visual) {
                this.counters.success++;
            }
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

    _finalizeVisualTest: function(test) {
        setTimeout(function() {
            var sanitizedName = sanitize(test.name);

            shot("gr/comparison/" + sanitizedName + ".png");

            if (this.saveVisualDiff) {
                shot("gr/base/" + OS.platform + "/" + sanitizedName + ".png");
            }

            clear();

            if (this.visualTests[sanitizedName]) {
                throw new Error("Duplicate unit-tests name " + test.name + "(sanitized name is " + sanitizedName + ")");
            }

            this.visualTests[sanitizedName] = test;
            this.hasVisualTests = true;

            this._reportTest(true, visual=true);
        }.bind(this), 16 * 4);
    },

    _runSkDiff: function() {
        var baseDir = __dirname + "jsunittest/";
        var cmd = baseDir + "gr/skdiff " +
                    "--listfilenames ";
        if (this.testRegex) {
            cmd += "--match " + this.testRegex + " ";
        }
        cmd += baseDir + "gr/base/" + OS.platform + "/ " + 
               baseDir + "gr/comparison/ " + 
               baseDir + "gr/results/";

        var out = window.exec(cmd);
        var lines = out.split("\n");
        var k = 0;
        var results = {};
        var processReport = false;

        for (line of lines) {
            if (/^\[/.test(line)) {
                this.visualTestsReport += "    " + line + "\n";

                var tmp = line.substr(line.indexOf(":") + 2).split(" ");
                for (file of tmp) {
                    if (!file) continue;
                    var name = file.replace(".png", "")

                    results[name] = k;

                    if (k > 0) processReport = true;
                }

                k++;
            }
        }

        if (processReport) {
            var f = new File(baseDir + "/gr/results/index.html", {encoding: "utf8"});
            f.openSync("r+");
            var data = f.readSync();
            data = data.replace(/="([^"]+)tests\/jsunittest\/gr\/([^"]+)"/gm, "=\"../$2\"");
            f.seekSync(0);
            f.writeSync(data);
            f.closeSync();
        }

        this._processSkDiff(results);
    },

    _processSkDiff: function(results) {
        // {{{ Utils (fail & success)
        var self = this;
        function fail(msg) {
            log.error(msg + "\n");
            log.error("\r[  ERROR  ]");
            console.write("\n");
            self.counters.fails++;
            self.failedTests.push(test);
        }

        function success() {
            log.success("\r[ SUCCESS ]");
            console.write("\n");
            self.counters.success++;
        }
        // }}}

        for (name in this.visualTests) {
            var test = this.visualTests[name];
            var state = results[name];

            console.write("[ RUNNING ] " + test.name + "\n");

            switch (state) {
                // Bits match
                case 0:
                    success();
                break;
                // Pixels match
                case 1:
                    fail("Pixels match, but bits doesn't");
                break;
                // Different pixels
                case 2:
                    fail("Pixels doesn't match");
                break;
                // Different dimensions
                case 3:
                    fail("Dimension doesn't match");
                break;
                // Comparison failed
                case 4:
                    fail("Comparison failed");
                break;
                // Not compared
                case 5:
                case "undefined":
                    fail("Not compared");
                break;
                default:
                    fail("Unknown status returned " + state);
                break;
            }
        }
    },

    _nextTest: function() {
        var test = this.tests.shift();
        this.currentTest = test;

        if (!test) {
            // All tests are finished
            if (this.hasVisualTests) {
                // Run skdiff to create a report for visual unit-tests
                this._runSkDiff();
            }

            if (this.completionCallback) this.completionCallback();
            return;
        }

        if (test.file != this.lastFile) {
            console.write("-----------------------------\n");
            log.info("Running tests for " + test.file + "\n");
            console.write("-----------------------------\n");
            this.lastFile = test.file;
        }

        console.write("[ " + (test.visual ? "PREPARING" : "RUNNING") + " ] " + test.name);
        console.write("\n");

        /*
         * Tests are ran in a setImmediate() call, so _nextTest() will be the
         * last visible frame to the debugger. This is needed for the debugger,
         * because, to know that an exception hasn't been catched is to check 
         * if the frame is the last frame on the stack. 
         */
        var success = false;
        (function() {
            try {
                if (test.visual) {
                    clear(); // Clear canvas before if previous tests drawed something
                    test["function"](this._finalizeVisualTest.bind(this, test));
                } else {
                    test["function"](this._reportTest.bind(this, true));
                }
                success = true;
            } catch (err) {
                sucess = false;
            }
        }.bind(this))();

        if (!test.async || success === false) {
            this._reportTest(success);
        } else if (test.watchdogDelay) {
            if (this.currentTest == null) {
                // Test already finished in a sync way,
                // no need to setup a timer
                return;
            }
            this.testWatchdogTimer = setTimeout(function() {
                Assert(false, true, "Timeout reached");
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

        if (!success) {
            log.info("Failed tests :\n");
            for (var i = 0; i < this.failedTests.length; i++) {
                try {
                    console.write("    - " + this.failedTests[i].name + "\n");
                } catch (e) {
                    log.info(e);
                }
            }

            if (this.visualTestsReport) {
                log.info("Visual tests report : \n");
                console.write(this.visualTestsReport);
            }
        }

        if (exit) {
            process.exit(success ? 0 : 1);
        }

        return success;
    }
}

module.exports = TestsRunner;


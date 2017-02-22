# Copyright 2017 Nidium Inc. All rights reserved.
# Use of this source code is governed by a MIT license
# that can be found in the LICENSE file.

from dokumentor import *

NamespaceDoc("VM", """The VM object provides you an interface for running code inside SpiderMonkey sandbox""",
    [ SeeDoc( "VM.Compartment" ) ],
    NO_Examples,
    products=["Frontend", "Server"]
)

FunctionDoc("VM.run", """Compile and run JavaScript code.""",
    NO_Sees,
    [ExampleDoc( """var VM = require("VM");

VM.run("console.log('Hello world');"); // Print "Hello world"

// Variables from the global scope are available
var str = "Hello world";
VM.run("console.log(str);"); // Print "Hello world"

// Variables from the current scope are not shared
(function() {
    var x = 1;
    VM.run("console.log(x);"); // x is not defined
})();""",
), ExampleDoc( """var VM = require("VM");

// The scope arguments, enable you to provide a custom object
// that will expose all of his properties to the scope.
// Variables from the global object are still available, but
// the variables from the scope are resolved first.
var myScope = {"name", "Nidium"};
VM.run("console.log(`Hello ${name}`)", {"scope": myScope});

// The scope object will also capture variables defined during
// the execution (only var, no let or class)
VM.run("var x = 15", {"scope": myScope});
console.log(myScope.x);""",
), ExampleDoc( """var VM = require("VM");
// The sandbox option enable you to provide a custom javascript
// environement. By default the sandbox is completely empty and
// you do not have access to any object from the current environement
var mySandbox = {"console": console, "x": 1};
VM.run("console.log(x)", {"sandbox": mySandbox});

// Just like the scope the sandbox can be used to variables (including global variables)
VM.run("hello = 'world'", {"sandbox": mySandbox});
console.log(mySandbox.hello); // print world""",
)],
    IS_Static, IS_Public, IS_Fast,
    [   ParamDoc("script", "The code to execute", "string|Stream|ArrayBuffer", NO_Default, IS_Obligated ),
        ParamDoc("options", "Options", ObjectDoc([
            ("scope", "A JavaScript object that will be added to the scope", "Object"),
            ("filename", "Specifies the filename used in stack traces produced by this script", "string"),
            ("lineOffset", "Specifies the line number offset that is displayed in stack traces produced by this script", "integer"),
            ("sandbox", "An object representing a sandbox.", "Object"),
            ("debugger", "`true` if you want to initialize a Debugger object inside the sandbox. Default to `false`", "boolean")
            ]), NO_Default, IS_Optional)
    ],
    ReturnDoc("The completion value of evaluating the given code. If the completion value is empty, undefined is returned.", "any" )
)

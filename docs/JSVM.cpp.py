# Copyright 2017 Nidium Inc. All rights reserved.
# Use of this source code is governed by a MIT license
# that can be found in the LICENSE file.

from dokumentor import *
#As the Debugger is not residing in the same <a href="https://developer.mozilla.org/en-US/docs/Mozilla/Projects/SpiderMonkey/Compartmentshttps://developer.mozilla.org/en-US/docs/Mozilla/Projects/SpiderMonkey/Compartments">compartment</a> you need to use a DebuggerCompartment to initialize and execute code inside the Debugger""",

NamespaceDoc("VM", """The VM object provides you an interface for running code in sandboxed environement""",
    [ SeeDoc( "VM.Compartment" ) ],
    NO_Examples,
    products=["Frontend", "Server"]
)

NamespaceDoc("VM.Compartement", """A compartement represent a sandboxed environement for running your javascript. No standard JS object are initialized in it. It does not have access to the parent scope.""",
    NO_Sees,
    NO_Examples,
    products=["Frontend", "Server"]
)

ConstructorDoc("VM.Compartement", "Create a new compartement. The returned object can be used to share variable with the compartement",
    NO_Sees,
    [ExampleDoc( """var VM = require("VM");

    var myCompartment = new VM.Compartment();
    myCompartment.x = "Hello world";

    // Expose console object inside the compartment
    myCompartement.console = console;

    vm.run("console.log(x); var y = 'Hello from compartment';", {"compartement": myCompartment});

    console.log(myCompartment.y); // Hello from compartment""" )],
    returns=ReturnDoc( "Return a compartement", "VM.Compartment" ))

VMCommonOptions = [
    ("scope", "A JavaScript object that will be added to the scope", "Object"),
    ("filename", "Specifies the filename used in stack traces produced by this script", "string"),
    ("lineOffset", "Specifies the line number offset that is displayed in stack traces produced by this script", "integer"),
    ("compartement", "A compartement object representing a sandbox", "VM.Compartement")
]

FunctionDoc("VM.run", """Compile and run a string of JavaScript.

Important notes : Variables from the current scope <strong>are not</strong> shared""",
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
// You can completely sandbox your code with the compartement option.
// A compartement in an empty JavaScript environement, it does not
// have access to the parent scope nor the current global object

// Create a compartement, and expose the console object inside it
var myCompartment = new VM.Compartement({"console": console});
VM.run("console.log('Hello from compartment')", {"compartment": myCompartment});

// Just like the scope the compartement can be used to capture global variables
VM.run("var str = 'From compartment'", {"compartment": myCompartment});
console.log(myCompartement.str);""",
)],
    IS_Static, IS_Public, IS_Fast,
    [   ParamDoc("script", "The code to execute", "string", NO_Default, IS_Obligated ),
        ParamDoc("options", "Options", ObjectDoc(VMCommonOptions), NO_Default, IS_Optional)
    ],
    NO_Returns
)

VMCommonOptions.append(("bind", "Bind the function to the given object", "any"))
VMCommonOptions.append(("name", "Specifies the function name to used in stack traces produced by this script", "any"))
VMCommonOptions.append(("args", "An object with key/values pairs representing the arguments to forward to the function", "any"))
FunctionDoc("VM.runInFunction", """Compile and run a string of JavaScript inside a function.""",
    NO_Sees,
    [ExampleDoc( """var VM = require("VM");

VM.runInFunction("console.log(myArg);", {"args": {"myArg": "Hello World"}}));""",
)],
    IS_Static, IS_Public, IS_Fast,
    [   ParamDoc("script", "The code to execute inside a function", "string", NO_Default, IS_Obligated ),
        ParamDoc("options", "Options", ObjectDoc(VMCommonOptions), NO_Default, IS_Optional)
    ],
    NO_Returns
)
